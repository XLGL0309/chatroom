#include "../include/web.h"
#include "../include/utils.h"
#include "../include/user.h"
#include "../include/message.h"
#include <iostream>

// HTML页面
const std::string htmlLogin = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Chat Room Login</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .container { max-width: 400px; margin: 0 auto; padding: 20px; border: 1px solid #ccc; border-radius: 5px; }
        h1 { text-align: center; }
        input { width: 100%; padding: 10px; margin: 10px 0; box-sizing: border-box;}
        button { width: 100%; padding: 10px; background: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer;box-sizing: border-box;  }
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

const std::string htmlChat = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Chat Room</title>
    <style>
        * {box-sizing: border-box;}
        body { font-family: Arial, sans-serif; margin: 20px; }
        .container { max-width: 600px; margin: 0 auto; }
        h1 { text-align: center; }
        .user-info { text-align: right; margin-bottom: 20px; font-weight: bold; }
        .message-section, .send-section { margin: 20px 0; padding: 20px; border: 1px solid #ccc; border-radius: 5px; }
        input, textarea { width: 100%; padding: 10px; margin: 10px 0; resize: vertical}
        button { padding: 10px 20px; background: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer; }
        button:hover { background: #45a049; }
        #messages { margin-top: 10px; padding: 10px; border: 1px solid #eee; min-height: 200px;box-sizing: border-box;}
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

std::string handleHttpRequest(const std::string& request, const std::string& clientIP) {
    // 解析HTTP请求
    size_t methodEnd = request.find(" ");
    size_t pathEnd = request.find(" ", methodEnd + 1);
    std::string method = "";
    std::string path = "";
    if (methodEnd != std::string::npos && pathEnd != std::string::npos) {
        method = request.substr(0, methodEnd);
        path = request.substr(methodEnd + 1, pathEnd - methodEnd - 1);
    }
    
    // 只记录关键请求的日志
    if (path == "/login" || path == "/send" || path == "/view") {
        std::cout << "Request: " << path << " from " << clientIP << std::endl;
    }
    
    std::string response;

    if (method == "GET") {
        if (path == "/") {
            // 显示登录页面
            response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(htmlLogin.length()) + "\r\n\r\n" + htmlLogin;
        } else if (path.find("/view") == 0) {
            // 查看消息
            size_t usernamePos = path.find("?username=");
            std::string username = "";
            if (usernamePos != std::string::npos) {
                username = path.substr(usernamePos + 10);
                // 对用户名进行URL解码
                username = urlDecode(username);
            }
            
            std::string messagesHtml = "";
            auto messages = g_messageManager.getMessagesForUser(username);
            for (const auto& msg : messages) {
                messagesHtml += htmlEscape(msg.from) + ": " + htmlEscape(msg.content) + "<br>";
            }
            
            std::string chatPage = htmlChat;
            // 替换所有的 %USERNAME% 出现
            size_t userPos = chatPage.find("%USERNAME%");
            while (userPos != std::string::npos) {
                chatPage.replace(userPos, 10, username);
                userPos = chatPage.find("%USERNAME%", userPos + username.length());
            }
            // 替换消息
            size_t msgPos = chatPage.find("%MESSAGES%");
            if (msgPos != std::string::npos) {
                chatPage.replace(msgPos, 10, messagesHtml);
            }
            // 清除错误信息
            size_t errorPos = chatPage.find("%ERROR%");
            if (errorPos != std::string::npos) {
                chatPage.replace(errorPos, 7, "");
            }
            
            response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(chatPage.length()) + "\r\n\r\n" + chatPage;
        } else {
            response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 9\r\n\r\nNot Found";
        }
    } else if (method == "POST") {
        if (path == "/login") {
            // 处理登录
            size_t bodyPos = request.find("\r\n\r\n");
            if (bodyPos != std::string::npos) {
                std::string body = request.substr(bodyPos + 4);
                std::string username = parseFormData(body, "username");
                // 解码HTML实体，确保处理的是原始中文字符
                username = htmlEntityDecode(username);
                
                if (!username.empty() && isValidUsername(username)) {
                    bool allowLogin = g_userManager.addUser(username, clientIP);
                    
                    if (allowLogin) {
                        // 对用户名进行URL编码
                        std::string encodedUsername = urlEncode(username);
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
            if (bodyPos != std::string::npos) {
                std::string body = request.substr(bodyPos + 4);
                std::string from = parseFormData(body, "from");
                std::string to = parseFormData(body, "to");
                std::string content = parseFormData(body, "content");
                
                // 验证发送者用户名是否有效且与IP匹配
                bool validUser = g_userManager.isValidUser(from, clientIP);
                
                if (!from.empty() && !to.empty() && !content.empty() && validUser) {
                    // 检查是否给自己发消息
                    if (from == to) {
                        // 不能给自己发消息，返回错误页面
                        std::string chatPage = htmlChat;
                        // 替换用户名
                        size_t userPos = chatPage.find("%USERNAME%");
                        while (userPos != std::string::npos) {
                            chatPage.replace(userPos, 10, from);
                            userPos = chatPage.find("%USERNAME%", userPos + 10);
                        }
                        // 替换错误信息
                        size_t errorPos = chatPage.find("%ERROR%");
                        if (errorPos != std::string::npos) {
                            chatPage.replace(errorPos, 7, "<div class='error'>Error: You cannot send messages to yourself</div>");
                        }
                        // 替换消息
                        size_t msgPos = chatPage.find("%MESSAGES%");
                        if (msgPos != std::string::npos) {
                            std::string messagesHtml = "";
                            auto messages = g_messageManager.getMessagesForUser(from);
                            for (const auto& msg : messages) {
                                messagesHtml += htmlEscape(msg.from) + ": " + htmlEscape(msg.content) + "<br>";
                            }
                            chatPage.replace(msgPos, 10, messagesHtml);
                        }
                        response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(chatPage.length()) + "\r\n\r\n" + chatPage;
                    } else if (!g_userManager.userExists(to)) {
                        // 目标用户不存在，返回错误页面
                        std::string chatPage = htmlChat;
                        // 替换用户名
                        size_t userPos = chatPage.find("%USERNAME%");
                        while (userPos != std::string::npos) {
                            chatPage.replace(userPos, 10, from);
                            userPos = chatPage.find("%USERNAME%", userPos + 10);
                        }
                        // 替换错误信息
                        size_t errorPos = chatPage.find("%ERROR%");
                        if (errorPos != std::string::npos) {
                            chatPage.replace(errorPos, 7, "<div class='error'>Error: User '" + to + "' does not exist</div>");
                        }
                        // 替换消息
                        size_t msgPos = chatPage.find("%MESSAGES%");
                        if (msgPos != std::string::npos) {
                            std::string messagesHtml = "";
                            auto messages = g_messageManager.getMessagesForUser(from);
                            for (const auto& msg : messages) {
                                messagesHtml += htmlEscape(msg.from) + ": " + htmlEscape(msg.content) + "<br>";
                            }
                            chatPage.replace(msgPos, 10, messagesHtml);
                        }
                        response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(chatPage.length()) + "\r\n\r\n" + chatPage;
                    } else {
                        // 保存消息
                        g_messageManager.addMessage(from, to, content);
                        
                        // 对用户名进行URL编码
                        std::string encodedFrom;
                        for (char c : from) {
                            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                                encodedFrom += c;
                            } else {
                                char hex[3];
                                sprintf(hex, "%02X", (unsigned char)c);
                                encodedFrom += "%" + std::string(hex);
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

    return response;
}