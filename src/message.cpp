/*
 * message.cpp
 * 消息管理功能的实现文件
 * 功能：实现消息管理器的方法，用于管理消息的添加、获取和清理
 */

#include "../include/message.h"
#include "../include/database.h"
#include <iostream>
#include <ctime>

#ifdef _WIN32
// Windows 平台的时间解析函数
#include <windows.h>
#include <string>

/**
 * 模拟 strptime 函数
 * 功能：解析时间字符串为 tm 结构
 * 参数：s - 时间字符串
 *       f - 格式字符串
 *       tm - 输出参数，存储解析结果
 * 返回值：解析后字符串的下一个位置
 */
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

/**
 * Message构造函数
 * 功能：初始化消息结构体，设置当前时间为时间戳
 */
Message::Message() : timestamp(time(nullptr)) {}

/**
 * MessageManager构造函数
 * 功能：初始化消息管理器
 */
MessageManager::MessageManager() {
}

/**
 * MessageManager析构函数
 */
MessageManager::~MessageManager() {
}

/**
 * 添加消息
 * 功能：添加一条新消息到数据库
 * 参数：from - 发送者
 *       to - 接收者
 *       content - 消息内容
 */
void MessageManager::addMessage(const std::string& from, const std::string& to, const std::string& content) {
    // 使用预处理语句插入新消息
    std::string insertQuery = "INSERT INTO messages (from_user, to_user, content) VALUES (?, ?, ?)";
    std::vector<std::string> insertParams = {from, to, content};
    DatabaseManager::getInstance().executePreparedUpdate(insertQuery, insertParams);
}

/**
 * 获取用户的消息
 * 功能：从数据库获取指定用户的所有消息
 * 参数：username - 用户名
 * 返回值：消息列表
 */
std::vector<Message> MessageManager::getMessagesForUser(const std::string& username) {
    std::vector<Message> messages;
    // 查询用户的所有消息（过期消息由数据库事件清理）
    std::string query = "SELECT from_user, to_user, content, timestamp FROM messages WHERE to_user = ? ORDER BY timestamp ASC";
    std::vector<std::string> queryParams = {username};
    MYSQL_RES* result = DatabaseManager::getInstance().executePreparedQuery(query, queryParams);
    if (result) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result)) != nullptr) {
            Message msg;
            msg.from = row[0] ? row[0] : "";
            msg.to = row[1] ? row[1] : "";
            msg.content = row[2] ? row[2] : "";
            // 处理时间戳
            if (row[3]) {
                // 尝试解析MySQL时间戳字符串
                struct tm tm_time;
                if (strptime(row[3], "%Y-%m-%d %H:%M:%S", &tm_time)) {
                    msg.timestamp = mktime(&tm_time);
                } else {
                    msg.timestamp = time(nullptr);
                }
            } else {
                msg.timestamp = time(nullptr);
            }
            messages.emplace_back(msg);
        }
        mysql_free_result(result);
    }
    return messages;
}

/**
 * 清理过期消息
 * 功能：清理所有超过24小时的消息
 */
void MessageManager::cleanExpiredMessages() {
    // 清理所有过期消息（超过24小时）
    DatabaseManager::getInstance().executePreparedUpdate(
        "DELETE FROM messages WHERE timestamp < DATE_SUB(NOW(), INTERVAL 24 HOUR)",
        {}
    );
}

/**
 * 清理用户的过期消息
 * 功能：清理指定用户超过24小时的消息
 * 参数：username - 用户名
 */
void MessageManager::cleanExpiredMessagesForUser(const std::string& username) {
    // 使用预处理语句清理指定用户的过期消息（超过24小时）
    std::string deleteQuery = "DELETE FROM messages WHERE to_user = ? AND timestamp < DATE_SUB(NOW(), INTERVAL 24 HOUR)";
    std::vector<std::string> deleteParams = {username};
    DatabaseManager::getInstance().executePreparedUpdate(deleteQuery, deleteParams);
}

/**
 * 获取单例实例
 * 功能：获取消息管理器的单例实例
 * 返回值：MessageManager的引用
 */
MessageManager& MessageManager::getInstance() {
    static MessageManager instance;
    return instance;
}