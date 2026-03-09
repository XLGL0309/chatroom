#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <mutex>
#include <thread>

// 跨平台Socket支持
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#define WSADATA int
#define WSAStartup(a, b) 0
#define WSACleanup() 0
#endif

using namespace std;

// 全局互斥锁（保护userMap和messageList）
std::mutex chatMutex;

struct User {
    string username;
    string ip;
};

struct Message {
    string from;
    string to;
    string content;
    time_t timestamp; // 添加时间戳，用于清理过期消息

    Message() : timestamp(time(nullptr)) {}
};

map<string, User> userMap; // 用户名到用户信息的映射
vector<Message> messageList;   // 存储所有消息

string htmlLogin = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Chat Room Login</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .container { max-width: 400px; margin: 0 auto; padding: 20px; border: 1px solid #ccc; border-radius: 5px; }
        h1 { text-align: center; }
        input { width: 100%; padding: 10px; margin: 10px 0; }
        button { width: 100%; padding: 10px; background: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer; }
        button:hover { background: #45a049; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Chat Room Login</h1>
        <form action="/login" method="post">
            <input type="text" name="username" placeholder="Enter username" required>
            <button type="submit">Login</button>
        </form>
    </div>
</body>
</html>
)";

string htmlChat = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Chat Room</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .container { max-width: 600px; margin: 0 auto; }
        h1 { text-align: center; }
        .user-info { text-align: right; margin-bottom: 20px; font-weight: bold; }
        .message-section, .send-section { margin: 20px 0; padding: 20px; border: 1px solid #ccc; border-radius: 5px; }
        input, textarea { width: 100%; padding: 10px; margin: 10px 0; }
        button { padding: 10px 20px; background: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer; }
        button:hover { background: #45a049; }
        #messages { margin-top: 10px; padding: 10px; border: 1px solid #eee; min-height: 200px; }
        .error { color: red; margin: 10px 0; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Chat Room</h1>
        <div class="user-info">Welcome, %USERNAME%</div>
        
        <div class="send-section">
            <h2>Send Message</h2>
            <form action="/send" method="post">
                <input type="hidden" name="from" value="%USERNAME%">
                <input type="text" name="to" placeholder="Recipient username" required>
                <textarea name="content" placeholder="Message content" rows="3" required></textarea>
                <button type="submit">Send Message</button>
                %ERROR%
            </form>
        </div>
        
        <div class="message-section">
            <h2>View Messages</h2>
            <form action="/view" method="get">
                <input type="hidden" name="username" value="%USERNAME%">
                <button type="submit">View My Messages</button>
            </form>
            <div id="messages">%MESSAGES%</div>
        </div>
    </div>
</body>
</html>
)";

string getContentType(const string& path) {
    if (path.find(".html") != string::npos) return "text/html";
    if (path.find(".css") != string::npos) return "text/css";
    if (path.find(".js") != string::npos) return "application/javascript";
    return "text/plain";
}

// HTML转义函数，防止XSS攻击
string htmlEscape(const string& str) {
    string escaped;
    for (char c : str) {
        switch (c) {
            case '<': escaped += "&lt;"; break;
            case '>': escaped += "&gt;"; break;
            case '&': escaped += "&amp;"; break;
            case '"': escaped += "&quot;"; break;
            case '\'': escaped += "&#39;"; break;
            default: escaped += c;
        }
    }
    return escaped;
}

// 验证用户名是否合法（允许：字母、数字、下划线、UTF-8中文）
bool isValidUsername(const string& username) {
    if (username.empty()) {
        return false;
    }

    size_t charCount = 0; // 字符数（不是字节数）
    size_t i = 0;
    while (i < username.length()) {
        unsigned char c = static_cast<unsigned char>(username[i]);

        if (c <= 0x7F) {
            // 1. 单字节：ASCII 字符
            if (!isalnum(c) && c != '_') {
                return false; // 只允许字母、数字、下划线
            }
            i++;
            charCount++;
        } else if ((c & 0xE0) == 0xE0) {
            // 2. 三字节：UTF-8 中文 (基本覆盖中日韩统一表意文字)
            if (i + 2 >= username.length()) return false; // 不完整的UTF-8序列
            
            unsigned char c2 = static_cast<unsigned char>(username[i+1]);
            unsigned char c3 = static_cast<unsigned char>(username[i+2]);
            
            // 检查后续字节是否符合 UTF-8 规范 (10xxxxxx)
            if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80) {
                return false;
            }
            
            // (可选) 严格限制在基本中文区间 U+4E00 到 U+9FFF
            // unsigned int codepoint = ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
            // if (codepoint < 0x4E00 || codepoint > 0x9FFF) return false;
            
            i += 3;
            charCount++;
        } else {
            // 3. 其他字节（双字节、四字节 emoji 等）：不允许
            return false;
        }

        // 限制最多 15 个字符（中文或英文都算一个字符）
        if (charCount > 15) {
            return false;
        }
    }
    return true;
}

// 统一的URL解码函数
string urlDecode(const string& str) {
    string decoded;
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == '%' && i + 2 < str.length()) {
            char hex[3] = {str[i+1], str[i+2], 0};
            decoded += static_cast<char>(strtol(hex, nullptr, 16));
            i += 2;
        } else if (str[i] == '+') {
            decoded += ' ';
        } else {
            decoded += str[i];
        }
    }
    return decoded;
}

// HTML实体解码函数
string htmlEntityDecode(const string& str) {
    string decoded;
    size_t i = 0;
    while (i < str.length()) {
        if (str[i] == '&' && i + 1 < str.length()) {
            // 检查是否是HTML实体
            if (str[i+1] == '#') {
                // 数字实体，如 &#1234;
                size_t end = str.find(';', i);
                if (end != string::npos) {
                    string numStr = str.substr(i + 2, end - i - 2);
                    try {
                        int code = stoi(numStr);
                        if (code >= 0 && code <= 0xFFFF) {
                            // 处理UTF-8编码
                            if (code <= 0x7F) {
                                // ASCII
                                decoded += static_cast<char>(code);
                            } else if (code <= 0x7FF) {
                                // 双字节UTF-8
                                decoded += static_cast<char>(0xC0 | (code >> 6));
                                decoded += static_cast<char>(0x80 | (code & 0x3F));
                            } else {
                                // 三字节UTF-8
                                decoded += static_cast<char>(0xE0 | (code >> 12));
                                decoded += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
                                decoded += static_cast<char>(0x80 | (code & 0x3F));
                            }
                            i = end + 1;
                            continue;
                        }
                    } catch (...) {
                        // 解析失败，按普通字符串处理
                    }
                }
            } else {
                // 命名实体，如 &lt;，这里简单处理，只支持常见的几个
                if (str.substr(i, 4) == "&lt;") {
                    decoded += '<';
                    i += 4;
                    continue;
                } else if (str.substr(i, 4) == "&gt;") {
                    decoded += '>';
                    i += 4;
                    continue;
                } else if (str.substr(i, 5) == "&amp;") {
                    decoded += '&';
                    i += 5;
                    continue;
                } else if (str.substr(i, 6) == "&quot;") {
                    decoded += '"';
                    i += 6;
                    continue;
                } else if (str.substr(i, 5) == "&#39;") {
                    decoded += '\'';
                    i += 5;
                    continue;
                }
            }
        }
        // 不是HTML实体，直接添加
        decoded += str[i];
        i++;
    }
    return decoded;
}

string parseFormData(const string& data, const string& key) {
    size_t pos = data.find(key + "=");
    if (pos == string::npos) return "";
    pos += key.length() + 1;
    size_t end = data.find("&", pos);
    if (end == string::npos) end = data.length();
    string value = data.substr(pos, end - pos);
    // 使用统一的URL解码函数
    return urlDecode(value);
}

void handleRequest(SOCKET clientSocket, const string& clientIP) {
    char buffer[4096];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        if (bytesRead < 0) {
            cerr << "recv failed: " << WSAGetLastError() << endl;
        }
        closesocket(clientSocket);
        return;
    }

    string request(buffer, bytesRead);
    cout << "Received request from " << clientIP << endl;
    cout << request << endl;

    // 解析HTTP请求
    size_t methodEnd = request.find(" ");
    size_t pathEnd = request.find(" ", methodEnd + 1);
    string method = request.substr(0, methodEnd);
    string path = request.substr(methodEnd + 1, pathEnd - methodEnd - 1);
    string response;

    if (method == "GET") {
        if (path == "/") {
            // 显示登录页面
            response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + to_string(htmlLogin.length()) + "\r\n\r\n" + htmlLogin;
        } else if (path.find("/view") == 0) {
            // 查看消息
            size_t usernamePos = path.find("?username=");
            string username = "";
            if (usernamePos != string::npos) {
                username = path.substr(usernamePos + 10);
                // 对用户名进行URL解码
                username = urlDecode(username);
            }
            
            string messagesHtml = "";
            {
                std::lock_guard<std::mutex> lock(chatMutex);
                // 清理过期消息（超过24小时）
                time_t now = time(nullptr);
                auto it = messageList.begin();
                while (it != messageList.end()) {
                    if (now - it->timestamp > 24 * 3600) {
                        it = messageList.erase(it);
                    } else {
                        ++it;
                    }
                }
                
                // 显示用户的消息
                for (const auto& msg : messageList) {
                    if (msg.to == username) {
                        messagesHtml += htmlEscape(msg.from) + ": " + htmlEscape(msg.content) + "<br>";
                    }
                }
            }
            
            string chatPage = htmlChat;
            // 替换所有的 %USERNAME% 出现
            size_t userPos = chatPage.find("%USERNAME%");
            while (userPos != string::npos) {
                chatPage.replace(userPos, 10, username);
                userPos = chatPage.find("%USERNAME%", userPos + username.length());
            }
            // 替换消息
            size_t msgPos = chatPage.find("%MESSAGES%");
            if (msgPos != string::npos) {
                chatPage.replace(msgPos, 10, messagesHtml);
            }
            // 清除错误信息
            size_t errorPos = chatPage.find("%ERROR%");
            if (errorPos != string::npos) {
                chatPage.replace(errorPos, 7, "");
            }
            
            response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + to_string(chatPage.length()) + "\r\n\r\n" + chatPage;
        } else {
            response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 9\r\n\r\nNot Found";
        }
    } else if (method == "POST") {
        if (path == "/login") {
            // 处理登录
            size_t bodyPos = request.find("\r\n\r\n");
            if (bodyPos != string::npos) {
                string body = request.substr(bodyPos + 4);
                string username = parseFormData(body, "username");
                // 解码HTML实体，确保处理的是原始中文字符
                username = htmlEntityDecode(username);
                
                if (!username.empty() && isValidUsername(username)) {
                    bool allowLogin = false;
                    {
                        std::lock_guard<std::mutex> lock(chatMutex);
                        // 检查用户名是否已存在
                        auto it = userMap.find(username);
                        if (it == userMap.end()) {
                            // 用户名不存在，允许登录
                            User user;
                            user.username = username;
                            user.ip = clientIP;
                            userMap[username] = user;
                            allowLogin = true;
                        } else {
                            // 用户名已存在，检查IP是否匹配
                            if (it->second.ip == clientIP) {
                                // IP匹配，允许登录
                                allowLogin = true;
                            } else {
                                // IP不匹配，拒绝登录
                                allowLogin = false;
                            }
                        }
                    }
                    
                    if (allowLogin) {
                        // 对用户名进行URL编码
                        string encodedUsername;
                        for (char c : username) {
                            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                                encodedUsername += c;
                            } else {
                                char hex[3];
                                sprintf(hex, "%02X", (unsigned char)c);
                                encodedUsername += "%" + string(hex);
                            }
                        }
                        // 跳转到聊天页面
                        response = "HTTP/1.1 302 Found\r\nLocation: /view?username=" + encodedUsername + "\r\n\r\n";
                    } else {
                        // 用户名已被其他IP使用，返回错误
                        response = "HTTP/1.1 403 Forbidden\r\nContent-Type: text/plain\r\nContent-Length: 30\r\n\r\nUsername is already used by another device";
                    }
                } else {
                    response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\nContent-Length: 26\r\n\r\nInvalid username format";
                }
            } else {
                response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\nContent-Length: 11\r\n\r\nBad Request";
            }
        } else if (path == "/send") {
            // 处理发送消息
            size_t bodyPos = request.find("\r\n\r\n");
            if (bodyPos != string::npos) {
                string body = request.substr(bodyPos + 4);
                string from = parseFormData(body, "from");
                string to = parseFormData(body, "to");
                string content = parseFormData(body, "content");
                
                // 验证发送者用户名是否有效且与IP匹配
                bool validUser = false;
                {
                    std::lock_guard<std::mutex> lock(chatMutex);
                    auto it = userMap.find(from);
                    if (it != userMap.end() && it->second.ip == clientIP) {
                        validUser = true;
                    }
                }
                
                if (!from.empty() && !to.empty() && !content.empty()) {
                    // 检查是否给自己发消息
                    if (from == to) {
                        // 不能给自己发消息，返回错误页面
                        string chatPage = htmlChat;
                        // 替换用户名
                size_t userPos = chatPage.find("%USERNAME%");
                while (userPos != string::npos) {
                    chatPage.replace(userPos, 10, from);
                    userPos = chatPage.find("%USERNAME%", userPos + 10);
                }
                        // 替换错误信息
                        size_t errorPos = chatPage.find("%ERROR%");
                        if (errorPos != string::npos) {
                            chatPage.replace(errorPos, 7, "<div class='error'>Error: You cannot send messages to yourself</div>");
                        }
                        // 替换消息
                        size_t msgPos = chatPage.find("%MESSAGES%");
                        if (msgPos != string::npos) {
                            string messagesHtml = "";
                            {
                                std::lock_guard<std::mutex> lock(chatMutex);
                                // 清理过期消息（超过24小时）
                                time_t now = time(nullptr);
                                auto it = messageList.begin();
                                while (it != messageList.end()) {
                                    if (now - it->timestamp > 24 * 3600) {
                                        it = messageList.erase(it);
                                    } else {
                                        ++it;
                                    }
                                }
                                
                                // 显示用户的消息
                                for (const auto& msg : messageList) {
                                    if (msg.to == from) {
                                        messagesHtml += htmlEscape(msg.from) + ": " + htmlEscape(msg.content) + "<br>";
                                    }
                                }
                            }
                            chatPage.replace(msgPos, 10, messagesHtml);
                        }
                        response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + to_string(chatPage.length()) + "\r\n\r\n" + chatPage;
                    } else if (userMap.find(to) == userMap.end()) {
                        // 目标用户不存在，返回错误页面
                        string chatPage = htmlChat;
                        // 替换用户名
                size_t userPos = chatPage.find("%USERNAME%");
                while (userPos != string::npos) {
                    chatPage.replace(userPos, 10, from);
                    userPos = chatPage.find("%USERNAME%", userPos + 10);
                }
                        // 替换错误信息
                        size_t errorPos = chatPage.find("%ERROR%");
                        if (errorPos != string::npos) {
                            chatPage.replace(errorPos, 7, "<div class='error'>Error: User '" + to + "' does not exist</div>");
                        }
                        // 替换消息
                        size_t msgPos = chatPage.find("%MESSAGES%");
                        if (msgPos != string::npos) {
                            string messagesHtml = "";
                            {
                                std::lock_guard<std::mutex> lock(chatMutex);
                                // 清理过期消息（超过24小时）
                                time_t now = time(nullptr);
                                auto it = messageList.begin();
                                while (it != messageList.end()) {
                                    if (now - it->timestamp > 24 * 3600) {
                                        it = messageList.erase(it);
                                    } else {
                                        ++it;
                                    }
                                }
                                
                                // 显示用户的消息
                                for (const auto& msg : messageList) {
                                    if (msg.to == from) {
                                        messagesHtml += htmlEscape(msg.from) + ": " + htmlEscape(msg.content) + "<br>";
                                    }
                                }
                            }
                            chatPage.replace(msgPos, 10, messagesHtml);
                        }
                        response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + to_string(chatPage.length()) + "\r\n\r\n" + chatPage;
                    } else {
                        // 保存消息
                        Message msg;
                        msg.from = from;
                        msg.to = to;
                        msg.content = content; // 存储原始内容，不进行转义
                        {
                            std::lock_guard<std::mutex> lock(chatMutex);
                            messageList.push_back(msg);
                        }
                        
                        // 对用户名进行URL编码
                        string encodedFrom;
                        for (char c : from) {
                            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                                encodedFrom += c;
                            } else {
                                char hex[3];
                                sprintf(hex, "%02X", (unsigned char)c);
                                encodedFrom += "%" + string(hex);
                            }
                        }
                        // 跳回查看消息页面
                        response = "HTTP/1.1 302 Found\r\nLocation: /view?username=" + encodedFrom + "\r\n\r\n";
                    }
                } else {
                    response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\nContent-Length: 11\r\n\r\nBad Request";
                }
            } else {
                response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\nContent-Length: 11\r\n\r\nBad Request";
            }
        } else {
            response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 9\r\n\r\nNot Found";
        }
    } else {
        response = "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/plain\r\nContent-Length: 17\r\n\r\nMethod Not Allowed";
    }

    int bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
    if (bytesSent < 0) {
        cerr << "send failed: " << WSAGetLastError() << endl;
    }
    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed" << endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed" << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8888);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Bind failed" << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        cerr << "Listen failed" << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server started on port 8888" << endl;
    cout << "Local IP address: " << inet_ntoa(serverAddr.sin_addr) << endl;

    while (true) {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Accept failed" << endl;
            continue;
        }

        string clientIP = inet_ntoa(clientAddr.sin_addr);
        cout << "Client connected: " << clientIP << endl;

        // 创建线程处理客户端请求
        std::thread([clientSocket, clientIP]() {
            handleRequest(clientSocket, clientIP);
        }).detach();
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}