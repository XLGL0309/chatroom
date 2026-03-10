#ifndef WEB_H
#define WEB_H

#include <string>

// HTML页面
extern const std::string htmlLogin;
extern const std::string htmlChat;

// HTTP请求处理
std::string handleHttpRequest(const std::string& request, const std::string& clientIP);

#endif // WEB_H