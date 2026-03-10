#include "../include/utils.h"
#include <cctype>
#include <unordered_map>
// HTML转义函数，防止XSS攻击
std::string htmlEscape(const std::string& str) {
    std::string escaped;
    for (char c : str) {
        switch (c) {
            case '<': escaped += "&lt;"; break;
            case '>': escaped += "&gt;"; break;
            case '&': escaped += "&amp;"; break;
            case '"': escaped += "&quot;"; break;
            case '\'': escaped += "&#39;"; break;
            default: escaped += c;
        }
    }
    return escaped;
}

// 验证用户名是否合法（允许：字母、数字、下划线、UTF-8中文）
bool isValidUsername(const std::string& username) {
    if (username.empty()) {
        return false;
    }

    size_t charCount = 0; // 字符数（不是字节数）
    size_t i = 0;
    while (i < username.length()) {
        unsigned char c = static_cast<unsigned char>(username[i]);

        if (c <= 0x7F) {
            // 1. 单字节：ASCII 字符
            if (!isalnum(c) && c != '_') {
                return false; // 只允许字母、数字、下划线
            }
            i++;
            charCount++;
        } else if ((c & 0xE0) == 0xE0) {
            // 2. 三字节：UTF-8 中文 (基本覆盖中日韩统一表意文字)
            if (i + 2 >= username.length()) return false; // 不完整的UTF-8序列
            
            unsigned char c2 = static_cast<unsigned char>(username[i+1]);
            unsigned char c3 = static_cast<unsigned char>(username[i+2]);
            
            // 检查后续字节是否符合 UTF-8 规范 (10xxxxxx)
            if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80) {
                return false;
            }
            
            i += 3;
            charCount++;
        } else {
            // 3. 其他字节（双字节、四字节 emoji 等）：不允许
            return false;
        }

        // 限制最多 15 个字符（中文或英文都算一个字符）
        if (charCount > 15) {
            return false;
        }
    }
    return true;
}

// URL解码函数
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

// URL编码函数
std::string urlEncode(const std::string& str) {
    std::string encoded;
    encoded.reserve(str.length() * 3); // 预分配空间，最坏情况下每个字符需要3个字符空间
    for (char c : str) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;
        } else {
            char hex[3];
            sprintf(hex, "%02X", (unsigned char)c);
            encoded += "%" + std::string(hex);
        }
    }
    return encoded;
}

// HTML实体解码函数
// 辅助函数：将Unicode码点转换为UTF-8编码并添加到字符串
void appendUtf8(std::string& str, int code) {
    if (code <= 0x7F) {
        // 1字节：ASCII
        str += static_cast<char>(code);
    } else if (code <= 0x7FF) {
        // 2字节UTF-8
        str += static_cast<char>(0xC0 | (code >> 6));
        str += static_cast<char>(0x80 | (code & 0x3F));
    } else if (code <= 0xFFFF) {
        // 3字节UTF-8
        str += static_cast<char>(0xE0 | (code >> 12));
        str += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
        str += static_cast<char>(0x80 | (code & 0x3F));
    } else if (code <= 0x10FFFF) {
        // 4字节UTF-8 (支持emoji等)
        str += static_cast<char>(0xF0 | (code >> 18));
        str += static_cast<char>(0x80 | ((code >> 12) & 0x3F));
        str += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
        str += static_cast<char>(0x80 | (code & 0x3F));
    }
}

std::string htmlEntityDecode(const std::string& str) {
    // 预分配空间，减少内存重新分配
    std::string decoded;
    decoded.reserve(str.length());
    
    // 命名实体映射表，提高查找效率
    static const std::unordered_map<std::string, std::string> entityMap = {
        {"lt", "<"},
        {"gt", ">"},
        {"amp", "&"},
        {"quot", "\""},
        {"#39", "'"},
        {"nbsp", " "},
        {"copy", "©"},
        {"reg", "®"},
        {"trade", "™"},
        {"cent", "¢"},
        {"pound", "£"},
        {"yen", "¥"},
        {"euro", "€"}
    };
    
    size_t i = 0;
    while (i < str.length()) {
        if (str[i] == '&' && i + 1 < str.length()) {
            // 检查是否是HTML实体
            if (str[i+1] == '#') {
                // 数字实体，如 &#1234;
                size_t end = str.find(';', i);
                if (end != std::string::npos) {
                    std::string numStr = str.substr(i + 2, end - i - 2);
                    try {
                        int code = std::stoi(numStr);
                        if (code >= 0 && code <= 0x10FFFF) {
                            // 处理UTF-8编码
                            appendUtf8(decoded, code);
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

// 解析表单数据
std::string parseFormData(const std::string& data, const std::string& key) {
    size_t pos = data.find(key + "=");
    if (pos == std::string::npos) return "";
    pos += key.length() + 1;
    size_t end = data.find("&", pos);
    if (end == std::string::npos) end = data.length();
    std::string value = data.substr(pos, end - pos);
    // 使用统一的URL解码函数
    return urlDecode(value);
}

// 获取内容类型
std::string getContentType(const std::string& path) {
    if (path.find(".html") != std::string::npos) return "text/html";
    if (path.find(".css") != std::string::npos) return "text/css";
    if (path.find(".js") != std::string::npos) return "application/javascript";
    return "text/plain";
}