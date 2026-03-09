#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

struct User {
    string username;
    string ip;
};

struct Message {
    string from;
    string to;
    string content;
};

map<string, User> userMap; // 用户名到用户信息的映射
vector<Message> messageList;   // 存储所有消息

string htmlLogin = R"(
<!DOCTYPE html>
<html>
<head>
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

string parseFormData(const string& data, const string& key) {
    size_t pos = data.find(key + "=");
    if (pos == string::npos) return "";
    pos += key.length() + 1;
    size_t end = data.find("&", pos);
    if (end == string::npos) end = data.length();
    string value = data.substr(pos, end - pos);
    // 简单的URL解码
    string decoded;
    for (size_t i = 0; i < value.length(); i++) {
        if (value[i] == '%' && i + 2 < value.length()) {
            char hex[3] = {value[i+1], value[i+2], 0};
            decoded += static_cast<char>(strtol(hex, nullptr, 16));
            i += 2;
        } else if (value[i] == '+') {
            decoded += ' ';
        } else {
            decoded += value[i];
        }
    }
    return decoded;
}

void handleRequest(SOCKET clientSocket, const string& clientIP) {
    char buffer[4096];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
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
                string decoded;
                for (size_t i = 0; i < username.length(); i++) {
                    if (username[i] == '%' && i + 2 < username.length()) {
                        char hex[3] = {username[i+1], username[i+2], 0};
                        decoded += static_cast<char>(strtol(hex, nullptr, 16));
                        i += 2;
                    } else if (username[i] == '+') {
                        decoded += ' ';
                    } else {
                        decoded += username[i];
                    }
                }
                username = decoded;
            }
            
            string messagesHtml = "";
            for (const auto& msg : messageList) {
                if (msg.to == username) {
                    messagesHtml += msg.from + ": " + msg.content + "<br>";
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
                
                if (!username.empty()) {
                    // 保存用户信息
                    User user;
                    user.username = username;
                    user.ip = clientIP;
                    userMap[username] = user;
                    
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
                    response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\nContent-Length: 11\r\n\r\nBad Request";
                }
            } else {
                response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\nContent-Length: 11\r\n\r\nBad Request";
            }
        } else if (path == "/send") {
            // 处理发送消息
            size_t bodyPos = request.find("\r\n\r\n");
            if (bodyPos != string::npos) {
                string body = request.substr(bodyPos + 4);
                string to = parseFormData(body, "to");
                string content = parseFormData(body, "content");
                
                // 不需要解码HTML实体，因为parseFormData已经处理了URL编码
                // 直接使用解码后的值即可
                
                // 从请求中获取发送者用户名（这里简化处理，实际应该从会话中获取）
                string from = "";
                for (const auto& pair : userMap) {
                    if (pair.second.ip == clientIP) {
                        from = pair.first;
                        break;
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
                            userPos = chatPage.find("%USERNAME%", userPos + from.length());
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
                            for (const auto& msg : messageList) {
                                if (msg.to == from) {
                                    messagesHtml += msg.from + ": " + msg.content + "<br>";
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
                            userPos = chatPage.find("%USERNAME%", userPos + from.length());
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
                            for (const auto& msg : messageList) {
                                if (msg.to == from) {
                                    messagesHtml += msg.from + ": " + msg.content + "<br>";
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
                        msg.content = content;
                        messageList.push_back(msg);
                        
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

    send(clientSocket, response.c_str(), response.length(), 0);
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
        CreateThread(nullptr, 0, [](LPVOID param) -> DWORD {
            SOCKET clientSocket = reinterpret_cast<SOCKET>(param);
            sockaddr_in clientAddr;
            int clientAddrSize = sizeof(clientAddr);
            getpeername(clientSocket, (sockaddr*)&clientAddr, &clientAddrSize);
            string clientIP = inet_ntoa(clientAddr.sin_addr);
            handleRequest(clientSocket, clientIP);
            return 0;
        }, reinterpret_cast<LPVOID>(clientSocket), 0, nullptr);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}