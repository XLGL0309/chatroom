/*
 * utils.h
 * 工具函数的头文件
 * 功能：提供各种工具函数，如验证、编码解码、HTTP响应生成等
 */

#ifndef UTILS_H
#define UTILS_H

#include <string>

/**
 * 验证用户名是否合法
 * 功能：检查用户名是否符合要求（允许：字母、数字、下划线、UTF-8中文）
 * 参数：username - 用户名
 * 返回值：合法返回true，不合法返回false
 */
bool isValidUsername(const std::string& username);

/**
 * URL解码函数
 * 功能：对URL编码的字符串进行解码
 * 参数：str - URL编码的字符串
 * 返回值：解码后的字符串
 */
std::string urlDecode(const std::string& str);

/**
 * URL编码函数
 * 功能：对字符串进行URL编码
 * 参数：str - 原始字符串
 * 返回值：URL编码后的字符串
 */
std::string urlEncode(const std::string& str);

/**
 * HTML转义函数
 * 功能：对HTML特殊字符进行转义，防止XSS攻击
 * 参数：str - 原始字符串
 * 返回值：转义后的字符串
 */
std::string htmlEscape(const std::string& str);

/**
 * JSON转义函数
 * 功能：对JSON特殊字符进行转义
 * 参数：str - 原始字符串
 * 返回值：转义后的字符串
 */
std::string jsonEscape(const std::string& str);

/**
 * HTML实体解码函数
 * 功能：对HTML实体进行解码
 * 参数：str - 包含HTML实体的字符串
 * 返回值：解码后的字符串
 */
std::string htmlEntityDecode(const std::string& str);

/**
 * 解析表单数据
 * 功能：从表单数据中解析指定键的值
 * 参数：data - 表单数据
 *       key - 键名
 * 返回值：解析出的值
 */
std::string parseFormData(const std::string& data, const std::string& key);

/**
 * 获取内容类型
 * 功能：根据文件路径获取MIME类型
 * 参数：path - 文件路径
 * 返回值：MIME类型字符串
 */
std::string getContentType(const std::string& path);

/**
 * 解析URL参数
 * 功能：从URL中解析指定参数的值
 * 参数：url - URL字符串
 *       key - 参数名
 * 返回值：解析出的参数值
 */
std::string parseUrlParam(const std::string& url, const std::string& key);

/**
 * 生成HTTP响应
 * 功能：生成标准的HTTP响应
 * 参数：statusCode - 状态码
 *       statusMessage - 状态消息
 *       contentType - 内容类型
 *       content - 响应内容
 *       location - 重定向地址（可选）
 * 返回值：完整的HTTP响应字符串
 */
std::string createHttpResponse(int statusCode, const std::string& statusMessage, const std::string& contentType, const std::string& content, const std::string& location = "");

#endif // UTILS_H