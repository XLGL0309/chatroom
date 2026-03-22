#include "../include/utils.h"
#include <cctype>
#include <unordered_map>
// HTML escape function to prevent XSS attacks
std::string htmlEscape(const std::string& str) {
    std::string escaped;
    for (char c : str) {
        switch (c) {
            case '<': escaped += "&lt;"; break;
            case '>': escaped += "&gt;"; break;
            case '&': escaped += "&amp;"; break;
            case '"': escaped += "&quot;"; break;
            case '\'': escaped += "&#39;"; break;
            default: escaped += c; break;
        }
    }
    return escaped;
}

// JSON转义辅助函数
std::string jsonEscape(const std::string& input) {
    std::string output;
    for (char c : input) {
        switch (c) {
            case '"': output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\b': output += "\\b"; break;
            case '\f': output += "\\f"; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            default:
                // 对于非ASCII字符，保持原样
                output += c;
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
    } else {
        // 非重定向响应需要添加Content-Type和Content-Length头
        response += "Content-Type: " + contentType + "; charset=utf-8\r\n";
        response += "Content-Length: " + std::to_string(content.length()) + "\r\n";
    }
    //添加空行
    response += "\r\n";
    
    // 非重定向响应需要添加响应体
    if (location.empty()) {
        response += content;
    }
    
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
            if (!isalnum(c) && c != '_') {
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
            
            i += 3;
            charCount++;
        } else {
            // 3. Other bytes (two-byte, four-byte emoji, etc.): not allowed
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
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == '%' && i + 2 < str.length()) {
            char hex[3] = {str[i+1], str[i+2], 0};
            decoded += static_cast<char>(strtol(hex, nullptr, 16));
            i += 2;
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
        if (isalnum(uc) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;
        } else {
            char hex[3];
            sprintf(hex, "%02X", uc);
            encoded += "%" + std::string(hex);
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
                        int code = std::stoi(numStr);
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
    size_t pos = data.find(key + "=");
    if (pos == std::string::npos) return "";
    pos += key.length() + 1;
    size_t end = data.find("&", pos);
    if (end == std::string::npos) end = data.length();
    std::string value = data.substr(pos, end - pos);
    // Use unified URL decode function
    return urlDecode(value);
}

// Get content type
std::string getContentType(const std::string& path) {
    if (path.find(".html") != std::string::npos) return "text/html";
    if (path.find(".css") != std::string::npos) return "text/css";
    if (path.find(".js") != std::string::npos) return "application/javascript";
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
    
    // 查找指定参数
    size_t paramPos = query.find(key + "=");
    if (paramPos == std::string::npos) {
        return "";
    }
    
    // 提取参数值
    size_t valueStart = paramPos + key.length() + 1;
    size_t valueEnd = query.find("&", valueStart);
    std::string value;
    
    if (valueEnd != std::string::npos) {
        value = query.substr(valueStart, valueEnd - valueStart);
    } else {
        value = query.substr(valueStart);
    }
    
    // URL解码
    return urlDecode(value);
}