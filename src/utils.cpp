/*
 * utils.cpp
 * 工具函数的实现文件
 * 功能：实现各种工具函数，如验证、编码解码、HTTP响应生成等
 */

#include "../include/utils.h"
#include <cctype>
#include <unordered_map>

/**
 * HTML转义函数
 * 功能：对HTML特殊字符进行转义，防止XSS攻击
 * 参数：str - 原始字符串
 * 返回值：转义后的字符串
 */
std::string htmlEscape(const std::string& str) {
    // 预分配内存
    std::string escaped;
    escaped.reserve(str.length() * 2);
    for (char c : str) {
        switch (c) {
            case '&': escaped += "&amp;"; break;
            case '<': escaped += "&lt;"; break;
            case '>': escaped += "&gt;"; break;
            case '"': escaped += "&quot;"; break;
            case '\'': escaped += "&#39;"; break;
            default: escaped += c; break;
        }
    }
    return escaped;
}

/**
 * JSON转义函数
 * 功能：对JSON特殊字符进行转义
 * 参数：input - 原始字符串
 * 返回值：转义后的字符串
 */
std::string jsonEscape(const std::string& input) {
    // 预分配内存
    std::string output;
    output.reserve(input.length() * 2);
    for (unsigned char c : input) {
        switch (c) {
            case '"': output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\b': output += "\\b"; break;
            case '\f': output += "\\f"; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            default:
                // 处理所有控制字符（U+0000到U+001F）
                if (c < 0x20) {
                    char hex[7]; // \uXXXX 需要6个字符
                    snprintf(hex, sizeof(hex), "\\u%04X", c);
                    output += hex;
                } else {
                    // 对于非ASCII字符，保持原样
                    output += c;
                }
                break;
        }
    }
    return output;
}

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
std::string createHttpResponse(int statusCode, const std::string& statusMessage, const std::string& contentType, const std::string& content, const std::string& location, bool keepAlive) {
    std::string response;
    response += "HTTP/1.1 " + std::to_string(statusCode) + " " + statusMessage + "\r\n";

    // 添加Connection头部
    if (keepAlive) {
        response += "Connection: keep-alive\r\n";
    } else {
        response += "Connection: close\r\n";
    }

    // 如果提供了location参数，则添加Location头（用于重定向）
    if (!location.empty()) {
        response += "Location: " + location + "\r\n";
        // 重定向响应也应该包含Content-Type和Content-Length
        response += "Content-Type: " + contentType + "; charset=utf-8\r\n";
        response += "Content-Length: " + std::to_string(content.length()) + "\r\n";
    } else {
        // 非重定向响应需要添加Content-Type和Content-Length头
        response += "Content-Type: " + contentType + "; charset=utf-8\r\n";
        response += "Content-Length: " + std::to_string(content.length()) + "\r\n";
    }
    //添加空行
    response += "\r\n";
    
    // 无论是否重定向，都添加响应体
    response += content;
    
    return response;
}

/**
 * 验证用户名是否合法
 * 功能：检查用户名是否符合要求（允许：字母、数字、下划线、UTF-8中文）
 * 参数：username - 用户名
 * 返回值：合法返回true，不合法返回false
 */
bool isValidUsername(const std::string& username) {
    if (username.empty()) {
        return false;
    }

    size_t charCount = 0; // 字符数（不是字节数）
    size_t i = 0;
    while (i < username.length()) {
        unsigned char c = static_cast<unsigned char>(username[i]);

        if (c <= 0x7F) {
            // 1. 单字节：ASCII字符
            // 直接检查ASCII范围，避免依赖locale
            if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')) {
                return false; // 只允许字母、数字和下划线
            }
            i++;
            charCount++;
        } else if ((c & 0xE0) == 0xE0) {
            // 2. 三字节：UTF-8中文（覆盖CJK统一表意文字）
            if (i + 2 >= username.length()) return false; // 不完整的UTF-8序列
            
            unsigned char c2 = static_cast<unsigned char>(username[i+1]);
            unsigned char c3 = static_cast<unsigned char>(username[i+2]);
            
            // 检查后续字节是否符合UTF-8规范（10xxxxxx）
            if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80) {
                return false;
            }
            
            // 验证是否为中文汉字（基本汉字范围：0x4E00-0x9FFF）
            int codePoint = ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
            if (!(codePoint >= 0x4E00 && codePoint <= 0x9FFF)) {
                return false;
            }
            
            i += 3;
            charCount++;
        } else if ((c & 0xF0) == 0xF0) {
            // 3. 四字节：UTF-8扩展中文
            if (i + 3 >= username.length()) return false; // 不完整的UTF-8序列
            
            unsigned char c2 = static_cast<unsigned char>(username[i+1]);
            unsigned char c3 = static_cast<unsigned char>(username[i+2]);
            unsigned char c4 = static_cast<unsigned char>(username[i+3]);
            
            // 检查后续字节是否符合UTF-8规范（10xxxxxx）
            if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80 || (c4 & 0xC0) != 0x80) {
                return false;
            }
            
            // 验证是否为扩展汉字（扩展汉字范围：0x10000-0x2A6DF）
            int codePoint = ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
            if (!(codePoint >= 0x10000 && codePoint <= 0x2A6DF)) {
                return false;
            }
            
            i += 4;
            charCount++;
        } else {
            // 4. 其他字节（双字节等）：不允许
            return false;
        }

        // 限制最大15个字符（中文或英文都算一个字符）
        if (charCount > 15) {
            return false;
        }
    }
    return true;
}

/**
 * URL解码函数
 * 功能：对URL编码的字符串进行解码
 * 参数：str - URL编码的字符串
 * 返回值：解码后的字符串
 */
std::string urlDecode(const std::string& str) {
    std::string decoded;
    decoded.reserve(str.size()); // 预分配内存，避免多次重新分配
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == '%' && i + 2 < str.length()) {
            // 检查是否为合法的十六进制字符
            if (isxdigit(static_cast<unsigned char>(str[i+1])) && 
                isxdigit(static_cast<unsigned char>(str[i+2]))) {
                char hex[3] = {str[i+1], str[i+2], 0};
                char* endptr = nullptr;
                unsigned char val = static_cast<unsigned char>(strtol(hex, &endptr, 16));
                if (endptr == hex + 2) { // 确保两位都被转换
                    decoded += static_cast<char>(val);
                    i += 2;
                } else {
                    // 如果转换失败，保持原样
                    decoded += str[i];
                }
            } else {
                // 如果不是合法的十六进制字符，保持原样
                decoded += str[i];
            }
        } else if (str[i] == '+') {
            decoded += ' ';
        } else {
            decoded += str[i];
        }
    }
    return decoded;
}

/**
 * URL编码函数
 * 功能：对字符串进行URL编码
 * 参数：str - 原始字符串
 * 返回值：URL编码后的字符串
 */
std::string urlEncode(const std::string& str) {
    std::string encoded;
    encoded.reserve(str.length() * 3); // 预分配空间，最坏情况下每个字符需要3个字符空间
    for (char c : str) {
        unsigned char uc = static_cast<unsigned char>(c);
        // 直接检查ASCII范围，避免依赖locale
        if ((uc >= 'a' && uc <= 'z') || (uc >= 'A' && uc <= 'Z') || (uc >= '0' && uc <= '9') || 
            c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;
        } else {
            char hex[4]; // 2个十六进制字符 + 结束符
            snprintf(hex, sizeof(hex), "%02X", uc); // 使用snprintf避免缓冲区溢出
            encoded += '%';
            encoded += hex;
        }
    }
    return encoded;
}

/**
 * HTML实体解码函数
 * 功能：对HTML实体进行解码
 * 参数：str - 包含HTML实体的字符串
 * 返回值：解码后的字符串
 */
std::string htmlEntityDecode(const std::string& str) {
    // 预分配空间以减少内存重分配
    std::string decoded;
    decoded.reserve(str.length());
    
    // 命名实体映射表，用于高效查找
    static const std::unordered_map<std::string, std::string> entityMap = {
        {"lt", "<"},
        {"gt", ">"},
        {"amp", "&"},
        {"quot", "\""},
        {"#39", "'"},
        {"nbsp", " "},
        {"copy", "\xc2\xa9"}, // ©
        {"reg", "\xc2\xae"},  // ®
        {"trade", "\xe2\x84\xa2"}, // ™
        {"cent", "\xc2\xa2"}, // ¢
        {"pound", "\xc2\xa3"}, // £
        {"yen", "\xc2\xa5"},  // ¥
        {"euro", "\xe2\x82\xac"} // €
    };
    
    size_t i = 0;
    while (i < str.length()) {
        if (str[i] == '&' && i + 1 < str.length()) {
            // 检查是否为HTML实体
            if (str[i+1] == '#') {
                // 数字实体，如 &#1234;
                size_t end = str.find(';', i);
                if (end != std::string::npos) {
                    std::string numStr = str.substr(i + 2, end - i - 2);
                    try {
                        // 指定基数为10，避免八进制解析错误
                        int code = std::stoi(numStr, nullptr, 10);
                        if (code >= 0 && code <= 0x10FFFF) {
                            // 处理UTF-8编码（使用lambda表达式）
                            auto appendUtf8 = [&decoded](int code) {
                                if (code <= 0x7F) {
                                    // 1 byte: ASCII
                                    decoded += static_cast<char>(code);
                                } else if (code <= 0x7FF) {
                                    // 2-byte UTF-8
                                    decoded += static_cast<char>(0xC0 | (code >> 6));
                                    decoded += static_cast<char>(0x80 | (code & 0x3F));
                                } else if (code <= 0xFFFF) {
                                    // 3-byte UTF-8
                                    decoded += static_cast<char>(0xE0 | (code >> 12));
                                    decoded += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
                                    decoded += static_cast<char>(0x80 | (code & 0x3F));
                                } else if (code <= 0x10FFFF) {
                                    // 4-byte UTF-8 (支持emoji等)
                                    decoded += static_cast<char>(0xF0 | (code >> 18));
                                    decoded += static_cast<char>(0x80 | ((code >> 12) & 0x3F));
                                    decoded += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
                                    decoded += static_cast<char>(0x80 | (code & 0x3F));
                                }
                            };
                            appendUtf8(code);
                            i = end + 1;
                            continue;
                        }
                    } catch (...) {
                        // 解析失败，按普通字符串处理
                    }
                }
            } else {
                // 命名实体，如 &lt;
                size_t end = str.find(';', i);
                if (end != std::string::npos) {
                    std::string entityName = str.substr(i + 1, end - i - 1);
                    auto it = entityMap.find(entityName);
                    if (it != entityMap.end()) {
                        // 找到匹配的实体
                        decoded += it->second;
                        i = end + 1;
                        continue;
                    }
                }
            }
        }
        // 不是HTML实体，直接添加
        decoded += str[i];
        i++;
    }
    return decoded;
}

/**
 * 解析表单数据
 * 功能：从表单数据中解析指定键的值
 * 参数：data - 表单数据
 *       key - 键名
 * 返回值：解析出的值
 */
std::string parseFormData(const std::string& data, const std::string& key) {
    size_t pos = 0;
    while (pos < data.length()) {
        // 找到当前参数的起始位置
        size_t paramStart = pos;
        // 找到参数结束位置（下一个&或字符串结束）
        size_t paramEnd = data.find("&", paramStart);
        if (paramEnd == std::string::npos) paramEnd = data.length();
        
        // 提取当前参数
        std::string param = data.substr(paramStart, paramEnd - paramStart);
        // 找到键值对的分隔符
        size_t equalsPos = param.find("=");
        if (equalsPos != std::string::npos) {
            std::string currentKey = param.substr(0, equalsPos);
            // 确保匹配完整的键名
            if (currentKey == key) {
                std::string value = param.substr(equalsPos + 1);
                // 使用统一的URL解码函数
                return urlDecode(value);
            }
        }
        
        // 移动到下一个参数
        pos = paramEnd + 1;
    }
    return "";
}

/**
 * 获取内容类型
 * 功能：根据文件路径获取MIME类型
 * 参数：path - 文件路径
 * 返回值：MIME类型字符串
 */
std::string getContentType(const std::string& path) {
    // 只检查文件扩展名，从最后一个点开始
    size_t dotPos = path.rfind('.');
    if (dotPos != std::string::npos) {
        std::string extension = path.substr(dotPos);
        if (extension == ".html") return "text/html";
        if (extension == ".css") return "text/css";
        if (extension == ".js") return "application/javascript";
    }
    return "text/plain";
}

/**
 * 解析URL参数
 * 功能：从URL中解析指定参数的值
 * 参数：url - URL字符串
 *       key - 参数名
 * 返回值：解析出的参数值
 */
std::string parseUrlParam(const std::string& url, const std::string& key) {
    // 查找查询参数开始位置
    size_t queryPos = url.find("?");
    if (queryPos == std::string::npos) {
        return "";
    }
    
    std::string query = url.substr(queryPos + 1);
    
    size_t pos = 0;
    while (pos < query.length()) {
        // 找到当前参数的起始位置
        size_t paramStart = pos;
        // 找到参数结束位置（下一个&或字符串结束）
        size_t paramEnd = query.find("&", paramStart);
        if (paramEnd == std::string::npos) paramEnd = query.length();
        
        // 提取当前参数
        std::string param = query.substr(paramStart, paramEnd - paramStart);
        // 找到键值对的分隔符
        size_t equalsPos = param.find("=");
        if (equalsPos != std::string::npos) {
            std::string currentKey = param.substr(0, equalsPos);
            // 确保匹配完整的键名
            if (currentKey == key) {
                std::string value = param.substr(equalsPos + 1);
                // URL解码
                return urlDecode(value);
            }
        }
        
        // 移动到下一个参数
        pos = paramEnd + 1;
    }
    
    return "";
}