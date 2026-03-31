/*
 * web.h
 * Web相关功能的头文件
 * 功能：定义HTML页面和HTTP请求处理函数
 */

#ifndef WEB_H
#define WEB_H

#include <string>

// HTML页面
extern const std::string htmlLogin; // 登录页面
extern const std::string htmlChat;  // 聊天页面

/**
 * 处理HTTP请求
 * 功能：根据HTTP请求的方法、路径和正文生成响应
 * 参数：method - HTTP方法（GET、POST等）
 *       path - 请求路径
 *       body - 请求正文
 *       keepAlive - 是否保持连接
 * 返回值：HTTP响应
 */
std::string handleHttpRequest(const std::string& method, const std::string& path, const std::string& body, bool keepAlive);

#endif // WEB_H