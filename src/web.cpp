#include "../include/web.h"
#include "../include/utils.h"
#include "../include/user.h"
#include "../include/message.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

// 读取文件内容的辅助函数
std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return content;
}

// HTML页面
const std::string htmlLogin = readFile("html/login.html");
const std::string htmlChat = readFile("html/chat.html");

// 生成消息JSON的辅助函数
std::string generateMessagesJson(const std::vector<Message>& messages) {
    std::string json = "{\"messages\": [";
    bool first = true;
    for (const auto& msg : messages) {
        if (!first) json += ", ";
        json += "{\"from\": \"" + htmlEscape(msg.from) + "\", \"content\": \"" + htmlEscape(msg.content) + "\"}";
        first = false;
    }
    json += "]}";
    return json;
}

// 生成页面的辅助函数（简化版）
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
            page.replace(statusPos, 8, "<div class=\"error\">错误：" + error + "</div>");
        } else if (status == "success") {
            page.replace(statusPos, 8, "<div style=\"color: green; margin: 10px 0;\">消息发送成功！</div>");
        } else {
            page.replace(statusPos, 8, "");
        }
    } 
    
    return page;
}

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
            std::string filePath = "html/login.html";
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
            // 处理API请求，返回JSON格式的消息（长轮询）
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
            
            // 长轮询等待时间（秒）
            const int maxWaitTime = 30;
            time_t startTime = time(nullptr);
            
            // 等待新消息
            while (true) {
                auto messages = g_messageManager.getMessagesForUser(username);
                if (messages.size() > lastCount) {
                    // 有新消息，立即返回
                    std::string json = generateMessagesJson(messages);
                    // 确保返回正确的Content-Type和CORS头
                    std::string responseWithHeaders = "HTTP/1.1 200 OK\r\n";
                    responseWithHeaders += "Content-Type: application/json; charset=utf-8\r\n";
                    responseWithHeaders += "Access-Control-Allow-Origin: *\r\n";
                    responseWithHeaders += "Content-Length: " + std::to_string(json.length()) + "\r\n";
                    responseWithHeaders += "\r\n";
                    responseWithHeaders += json;
                    response = responseWithHeaders;
                    break;
                }
                
                // 检查是否超时
                time_t currentTime = time(nullptr);
                if (currentTime - startTime >= maxWaitTime) {
                    break;
                }
                
                // 没有新消息，等待1秒后再次检查
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            
            // 超时后返回空消息
            if (response.empty()) {
                std::string json = "{\"messages\": []}";
                std::string responseWithHeaders = "HTTP/1.1 200 OK\r\n";
                responseWithHeaders += "Content-Type: application/json; charset=utf-8\r\n";
                responseWithHeaders += "Access-Control-Allow-Origin: *\r\n";
                responseWithHeaders += "Content-Length: " + std::to_string(json.length()) + "\r\n";
                responseWithHeaders += "\r\n";
                responseWithHeaders += json;
                response = responseWithHeaders;
            }
        } else {
            response = createHttpResponse(404, "Not Found", "text/plain", "页面不存在");
        }
    } else if (method == "POST") {
        if (path == "/login") {
            // 处理登录
            size_t bodyPos = request.find("\r\n\r\n");
            if (bodyPos != std::string::npos) {
                std::string body = request.substr(bodyPos + 4);
                std::string username = parseFormData(body, "username");
                std::string password = parseFormData(body, "password");
                // 解码HTML实体，确保处理的是原始中文字符
                username = htmlEntityDecode(username);
                
                if (!username.empty() && !password.empty() && isValidUsername(username) && password.length() >= 6) {
                    bool allowLogin = g_userManager.loginUser(username, password, clientIP);
                    
                    if (allowLogin) {
                        // 对用户名进行URL编码
                        std::string encodedUsername = urlEncode(username);
                        // 跳转到聊天页面
                        response = createHttpResponse(302, "Found", "", "", "/view?username=" + encodedUsername);
                    } else {
                        // 用户名或密码错误，返回错误
                        response = createHttpResponse(401, "Unauthorized", "text/plain", "用户名或密码错误");
                    }
                } else {
                    response = createHttpResponse(400, "Bad Request", "text/plain", "输入无效：用户名必须有效且密码至少6个字符");
                }
            } else {
                response = createHttpResponse(400, "Bad Request", "text/plain", "请求参数错误");
            }
        } else if (path == "/register") {
            // 处理注册
            size_t bodyPos = request.find("\r\n\r\n");
            if (bodyPos != std::string::npos) {
                std::string body = request.substr(bodyPos + 4);
                std::string username = parseFormData(body, "username");
                std::string password = parseFormData(body, "password");
                // 解码HTML实体，确保处理的是原始中文字符
                username = htmlEntityDecode(username);
                
                if (!username.empty() && !password.empty() && isValidUsername(username) && password.length() >= 6) {
                    bool registered = g_userManager.registerUser(username, password, clientIP);
                    
                    if (registered) {
                        // 注册成功，自动登录并跳转到聊天页面
                        std::string encodedUsername = urlEncode(username);
                        response = createHttpResponse(302, "Found", "", "", "/view?username=" + encodedUsername);
                    } else {
                        // 用户名已存在，返回错误
                        response = createHttpResponse(409, "Conflict", "text/plain", "用户名已存在");
                    }
                } else {
                    response = createHttpResponse(400, "Bad Request", "text/plain", "输入无效：用户名必须有效且密码至少6个字符");
                }
            } else {
                    response = createHttpResponse(400, "Bad Request", "text/plain", "请求参数错误");
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
                        std::string errorPage = generatePage(from, "", "你不能给自己发送消息");
                        response = createHttpResponse(200, "OK", "text/html", errorPage);
                    } else if (!g_userManager.userExists(to)) {
                        // 目标用户不存在，返回错误页面
                        std::string errorPage = generatePage(from, "", "用户 '" + to + "' 不存在");
                        response = createHttpResponse(200, "OK", "text/html", errorPage);
                    } else {
                        // 保存消息
                        g_messageManager.addMessage(from, to, content);
                        
                        // 对用户名进行URL编码
                        std::string encodedFrom = urlEncode(from);
                        // 跳回查看消息页面并添加成功提示
                        response = createHttpResponse(302, "Found", "", "", "/view?username=" + encodedFrom + "&status=success");
                    }
                } else {
                    response = createHttpResponse(400, "Bad Request", "text/plain", "请求参数错误");
                }
            } else {
                response = createHttpResponse(400, "Bad Request", "text/plain", "请求参数错误");
            }
        } else {
            response = createHttpResponse(404, "Not Found", "text/plain", "页面不存在");
        }
    } else {
        response = createHttpResponse(405, "Method Not Allowed", "text/plain", "不允许的请求方法");
    }

    return response;
}