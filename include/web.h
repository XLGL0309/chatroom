#ifndef WEB_H
#define WEB_H

#include <string>

// HTML页面
extern const std::string htmlLogin;
extern const std::string htmlChat;

// HTTP请求处理
std::string handleHttpRequest(const std::string& method, const std::string& path, const std::string& body);

#endif // WEB_H