#include "../include/utils.h"
#include <cctype>
#include <unordered_map>
// HTML escape function to prevent XSS attacks
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

// JSON转义辅助函数
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

// Generate HTTP response
std::string createHttpResponse(int statusCode, const std::string& statusMessage, const std::string& contentType, const std::string& content, const std::string& location) {
    std::string response;
    response += "HTTP/1.1 " + std::to_string(statusCode) + " " + statusMessage + "\r\n";
    
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

// Validate username (allowed: letters, numbers, underscore, UTF-8 Chinese)
bool isValidUsername(const std::string& username) {
    if (username.empty()) {
        return false;
    }

    size_t charCount = 0; // 字符数（不是字节数）
    size_t i = 0;
    while (i < username.length()) {
        unsigned char c = static_cast<unsigned char>(username[i]);

        if (c <= 0x7F) {
            // 1. Single byte: ASCII character
            // 直接检查ASCII范围，避免依赖locale
            if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')) {
                return false; // Only letters, numbers, and underscore are allowed
            }
            i++;
            charCount++;
        } else if ((c & 0xE0) == 0xE0) {
            // 2. Three bytes: UTF-8 Chinese (covers CJK unified ideographs)
            if (i + 2 >= username.length()) return false; // Incomplete UTF-8 sequence
            
            unsigned char c2 = static_cast<unsigned char>(username[i+1]);
            unsigned char c3 = static_cast<unsigned char>(username[i+2]);
            
            // Check if subsequent bytes follow UTF-8 specification (10xxxxxx)
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
            // 3. Four bytes: UTF-8 extended Chinese
            if (i + 3 >= username.length()) return false; // Incomplete UTF-8 sequence
            
            unsigned char c2 = static_cast<unsigned char>(username[i+1]);
            unsigned char c3 = static_cast<unsigned char>(username[i+2]);
            unsigned char c4 = static_cast<unsigned char>(username[i+3]);
            
            // Check if subsequent bytes follow UTF-8 specification (10xxxxxx)
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
            // 4. Other bytes (two-byte, etc.): not allowed
            return false;
        }

        // Limit to maximum 15 characters (Chinese or English both count as one character)
        if (charCount > 15) {
            return false;
        }
    }
    return true;
}

// URL decode function
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

// URL encode function
std::string urlEncode(const std::string& str) {
    std::string encoded;
    encoded.reserve(str.length() * 3); // Preallocate space, worst case each character needs 3 characters space
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

// HTML entity decode function
std::string htmlEntityDecode(const std::string& str) {
    // Preallocate space to reduce memory reallocation
    std::string decoded;
    decoded.reserve(str.length());
    
    // Named entity mapping table for efficient lookup
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
            // Check if it's an HTML entity
            if (str[i+1] == '#') {
                // Numeric entity, like &#1234;
                size_t end = str.find(';', i);
                if (end != std::string::npos) {
                    std::string numStr = str.substr(i + 2, end - i - 2);
                    try {
                        // 指定基数为10，避免八进制解析错误
                        int code = std::stoi(numStr, nullptr, 10);
                        if (code >= 0 && code <= 0x10FFFF) {
                            // Handle UTF-8 encoding (using lambda expression)
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
                                    // 4-byte UTF-8 (supports emoji, etc.)
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
                // Named entity, like &lt;
                size_t end = str.find(';', i);
                if (end != std::string::npos) {
                    std::string entityName = str.substr(i + 1, end - i - 1);
                    auto it = entityMap.find(entityName);
                    if (it != entityMap.end()) {
                        // Found matching entity
                        decoded += it->second;
                        i = end + 1;
                        continue;
                    }
                }
            }
        }
        // Not an HTML entity, add directly
        decoded += str[i];
        i++;
    }
    return decoded;
}

// Parse form data
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
                // Use unified URL decode function
                return urlDecode(value);
            }
        }
        
        // 移动到下一个参数
        pos = paramEnd + 1;
    }
    return "";
}

// Get content type
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

// Parse URL parameters
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