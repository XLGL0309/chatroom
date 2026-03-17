#include "../include/message.h"
#include "../include/database.h"
#include <iostream>
#include <ctime>

#ifdef _WIN32
// Windows 平台的时间解析函数
#include <windows.h>
#include <string>

// 模拟 strptime 函数
char* strptime(const char* s, const char* f, struct tm* tm) {
    int year, month, day, hour, minute, second;
    if (sscanf(s, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6) {
        tm->tm_year = year - 1900;
        tm->tm_mon = month - 1;
        tm->tm_mday = day;
        tm->tm_hour = hour;
        tm->tm_min = minute;
        tm->tm_sec = second;
        tm->tm_isdst = -1; // 让 mktime 自动确定夏令时
        return (char*)(s + 19); // 假设时间字符串长度为 19
    }
    return nullptr;
}
#endif

Message::Message() : timestamp(time(nullptr)) {}

void MessageManager::addMessage(const std::string& from, const std::string& to, const std::string& content) {
    // Insert new message using prepared statement
    std::string insertQuery = "INSERT INTO messages (from_user, to_user, content) VALUES (?, ?, ?)";
    std::vector<std::string> insertParams = {from, to, content};
    g_databaseManager.executePreparedUpdate(insertQuery, insertParams);
}

std::vector<Message> MessageManager::getMessagesForUser(const std::string& username) {
    std::vector<Message> messages;
    // Query all messages for user (expired messages are cleaned by database event)
    std::string query = "SELECT from_user, to_user, content, timestamp FROM messages WHERE to_user = ? ORDER BY timestamp ASC";
    std::vector<std::string> queryParams = {username};
    MYSQL_RES* result = g_databaseManager.executePreparedQuery(query, queryParams);
    if (result) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result)) != nullptr) {
            Message msg;
            msg.from = row[0] ? row[0] : "";
            msg.to = row[1] ? row[1] : "";
            msg.content = row[2] ? row[2] : "";
            // Handle timestamp
            if (row[3]) {
                // Try to parse MySQL timestamp string
                struct tm tm_time;
                if (strptime(row[3], "%Y-%m-%d %H:%M:%S", &tm_time)) {
                    msg.timestamp = mktime(&tm_time);
                } else {
                    msg.timestamp = time(nullptr);
                }
            } else {
                msg.timestamp = time(nullptr);
            }
            messages.push_back(msg);
        }
        mysql_free_result(result);
    }
    return messages;
}

void MessageManager::cleanExpiredMessages() {
    // Clean all expired messages (older than 24 hours)
    g_databaseManager.executePreparedUpdate(
        "DELETE FROM messages WHERE timestamp < DATE_SUB(NOW(), INTERVAL 24 HOUR)",
        {}
    );
}

void MessageManager::cleanExpiredMessagesForUser(const std::string& username) {
    // Clean expired messages for specific user (older than 24 hours) using prepared statement
    std::string deleteQuery = "DELETE FROM messages WHERE to_user = ? AND timestamp < DATE_SUB(NOW(), INTERVAL 24 HOUR)";
    std::vector<std::string> deleteParams = {username};
    g_databaseManager.executePreparedUpdate(deleteQuery, deleteParams);
}

// 全局消息管理器实例
MessageManager g_messageManager;