#ifndef UTILS_H
#define UTILS_H

#include <string>

// HTML转义函数，防止XSS攻击
std::string htmlEscape(const std::string& str);

// 验证用户名是否合法（允许：字母、数字、下划线、UTF-8中文）
bool isValidUsername(const std::string& username);

// URL解码函数
std::string urlDecode(const std::string& str);

// HTML实体解码函数
std::string htmlEntityDecode(const std::string& str);

// 解析表单数据
std::string parseFormData(const std::string& data, const std::string& key);

// 获取内容类型
std::string getContentType(const std::string& path);

#endif // UTILS_H