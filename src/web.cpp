/*
 * web.cpp
 * Web相关功能的实现文件
 * 功能：实现HTTP请求处理、页面生成和消息管理
 */

#include "../include/web.h"
#include "../include/utils.h"
#include "../include/user.h"
#include "../include/message.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

/**
 * 读取文件内容的辅助函数
 * 功能：读取指定文件的内容
 * 参数：filename - 文件路径
 * 返回值：文件内容
 */
std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return content;
}

// HTML页面
const std::string htmlLogin = readFile("./html/login.html");
const std::string htmlChat = readFile("./html/chat.html");

/**
 * 生成消息JSON的辅助函数
 * 功能：将消息列表转换为JSON格式
 * 参数：messages - 消息列表
 * 返回值：JSON格式的消息字符串
 */
std::string generateMessagesJson(const std::vector<Message>& messages) {
    std::string json = "{\"messages\": [";
    bool first = true;
    for (const auto& msg : messages) {
        if (!first) json += ", ";
        json += "{\"from\": \"" + jsonEscape(htmlEscape(msg.from)) + "\", \"content\": \"" + jsonEscape(htmlEscape(msg.content)) + "\"}";
        first = false;
    }
    json += "]}";
    return json;
}

/**
 * 生成页面的辅助函数
 * 功能：生成聊天页面，替换占位符
 * 参数：username - 用户名
 *       status - 状态信息
 *       error - 错误信息
 * 返回值：生成的HTML页面
 */
std::string generatePage(const std::string& username, const std::string& status = "", const std::string& error = "") {
    std::string page = htmlChat;
    
    // 替换所有的 %USERNAME% 出现
    size_t userPos = page.find("%USERNAME%");
    while (userPos != std::string::npos) {
        page.replace(userPos, 10, username);
        userPos = page.find("%USERNAME%", userPos + username.length());
    }
    
    // 替换消息占位符为空白，由JavaScript轮询填充
    size_t messagesPos = page.find("%MESSAGES%");
    if (messagesPos != std::string::npos) {
        page.replace(messagesPos, 10, "");
    }
    
    // 设置状态信息
    size_t statusPos = page.find("%STATUS%");
    if (statusPos != std::string::npos) {
        if (!error.empty()) {
            page.replace(statusPos, 8, "<div class=\"error\">Error: " + error + "</div>");
        } else if (status == "success") {
            page.replace(statusPos, 8, "<div style=\"color: green; margin: 10px 0;\">Message sent successfully!</div>");
        } else {
            page.replace(statusPos, 8, "");
        }
    } 
    
    return page;
}

/**
 * 处理HTTP请求
 * 功能：根据HTTP请求的方法、路径和正文生成响应
 * 参数：method - HTTP方法（GET、POST等）
 *       path - 请求路径
 *       body - 请求正文
 * 返回值：HTTP响应
 */
std::string handleHttpRequest(const std::string& method, const std::string& path, const std::string& body) {
    std::string response;

    if (method == "GET") {
        if (path == "/") {
            // 显示登录页面
            std::string filePath = "./html/login.html";
            std::string content = readFile(filePath);
            std::string contentType = getContentType(filePath);
            response = createHttpResponse(200, "OK", contentType, content);
        } else if (path.find("/view") == 0) {
            // 查看消息
            // 使用parseUrlParam函数解析URL参数
            std::string username = parseUrlParam(path, "username");
            std::string status = parseUrlParam(path, "status");
            std::string error = parseUrlParam(path, "error");
            
            // 生成聊天页面
            std::string chatPage = generatePage(username, status, error);
            response = createHttpResponse(200, "OK", "text/html", chatPage);
        } else if (path.find("/api/messages") == 0) {
            // 处理API请求，返回JSON格式的消息（短轮询）
            std::string username = parseUrlParam(path, "username");
            std::string lastMessageCount = parseUrlParam(path, "lastCount");
            int lastCount = 0;
            if (!lastMessageCount.empty()) {
                try {
                    lastCount = std::stoi(lastMessageCount);
                } catch (...) {
                    lastCount = 0;
                }
            }
            
            // 立即检查是否有新消息，不等待
            auto messages = MessageManager::getInstance().getMessagesForUser(username);
            // 无论是否有新消息，都返回所有消息
            std::string json = generateMessagesJson(messages);
            // 确保返回正确的Content-Type和CORS头
            std::string responseWithHeaders = "HTTP/1.1 200 OK\r\n";
            responseWithHeaders += "Content-Type: application/json; charset=utf-8\r\n";
            responseWithHeaders += "Access-Control-Allow-Origin: *\r\n";
            responseWithHeaders += "Content-Length: " + std::to_string(json.length()) + "\r\n";
            responseWithHeaders += "\r\n";
            responseWithHeaders += json;
            response = responseWithHeaders;
        } else {
            response = createHttpResponse(404, "Not Found", "text/plain", "Page not found");
        }
    } else if (method == "POST") {
        if (path == "/login") {
            // 处理登录
            if (!body.empty()) {
                std::string username = parseFormData(body, "username");
                std::string password = parseFormData(body, "password");
                // 解码HTML实体，确保处理的是原始中文字符
                username = htmlEntityDecode(username);
                
                if (!username.empty() && !password.empty() && isValidUsername(username) && password.length() >= 6) {
                    bool allowLogin = UserManager::getInstance().loginUser(username, password);
                    
                    if (allowLogin) {
                        // 对用户名进行URL编码
                        std::string encodedUsername = urlEncode(username);
                        // 跳转到聊天页面
                        response = createHttpResponse(302, "Found", "", "", "/view?username=" + encodedUsername);
                    } else {
                        // 用户名或密码错误，返回错误
                        response = createHttpResponse(401, "Unauthorized", "text/plain", "Invalid username or password");
                    }
                } else {
                    response = createHttpResponse(400, "Bad Request", "text/plain", "Invalid input: username must be valid and password must be at least 6 characters");
                }
            } else {
                response = createHttpResponse(400, "Bad Request", "text/plain", "Invalid request parameters ");
            }
        } else if (path == "/register") {
            // 处理注册
            if (!body.empty()) {
                std::string username = parseFormData(body, "username");
                std::string password = parseFormData(body, "password");
                // 解码HTML实体，确保处理的是原始中文字符
                username = htmlEntityDecode(username);
                
                if (!username.empty() && !password.empty() && isValidUsername(username) && password.length() >= 6) {
                    bool registered = UserManager::getInstance().registerUser(username, password);
                    
                    if (registered) {
                        // 注册成功，自动登录并跳转到聊天页面
                        std::string encodedUsername = urlEncode(username);
                        response = createHttpResponse(302, "Found", "", "", "/view?username=" + encodedUsername);
                    } else {
                        // 用户名已存在，返回错误
                        response = createHttpResponse(409, "Conflict", "text/plain", "Username already exists");
                    }
                } else {
                    response = createHttpResponse(400, "Bad Request", "text/plain", "Invalid input: username must be valid and password must be at least 6 characters");
                }
            } else {
                    response = createHttpResponse(400, "Bad Request", "text/plain", "Invalid request parameters");
            }
        } else if (path == "/send") {
            // 处理发送消息
            if (!body.empty()) {
                std::string from = parseFormData(body, "from");
                std::string to = parseFormData(body, "to");
                std::string content = parseFormData(body, "content");
                
                // 验证发送者用户名是否存在
                bool validUser = UserManager::getInstance().userExists(from);
                
                if (!from.empty() && !to.empty() && !content.empty() && validUser) {
                    // 检查消息长度
                    if (content.length() > 1000000) {
                        // 消息过长，返回错误页面
                        std::string errorPage = generatePage(from, "", "Message too long. Maximum length is 1,000,000 characters.");
                        response = createHttpResponse(200, "OK", "text/html", errorPage);
                    } else if (from == to) {
                        // 不能给自己发消息，返回错误页面
                        std::string errorPage = generatePage(from, "", "You cannot send messages to yourself");
                        response = createHttpResponse(200, "OK", "text/html", errorPage);
                    } else if (!UserManager::getInstance().userExists(to)) {
                        // 目标用户不存在，返回错误页面
                        std::string errorPage = generatePage(from, "", "User '" + to + "' does not exist");
                        response = createHttpResponse(200, "OK", "text/html", errorPage);
                    } else {
                        // 保存消息
                        MessageManager::getInstance().addMessage(from, to, content);
                        
                        // 对用户名进行URL编码
                        std::string encodedFrom = urlEncode(from);
                        // 跳回查看消息页面并添加成功提示
                        response = createHttpResponse(302, "Found", "", "", "/view?username=" + encodedFrom + "&status=success");
                    }
                } else {
                    response = createHttpResponse(400, "Bad Request", "text/plain", "Invalid request parameters");
                }
            } else {
                response = createHttpResponse(400, "Bad Request", "text/plain", "Invalid request parameters");
            }
        } else {
            response = createHttpResponse(404, "Not Found", "text/plain", "Page not found");
        }
    } else {
        response = createHttpResponse(405, "Method Not Allowed", "text/plain", "Method not allowed");
    }

    return response;
}