/*
 * user.cpp
 * 用户管理功能的实现文件
 * 功能：实现用户管理器的方法，用于用户注册、登录和检查用户是否存在
 */

#include "../include/user.h"
#include "../include/database.h"
#include <iostream>

/**
 * UserManager构造函数
 * 功能：初始化用户管理器
 */
UserManager::UserManager() {
}

/**
 * UserManager析构函数
 */
UserManager::~UserManager() {
}

/**
 * 注册用户
 * 功能：注册新用户
 * 参数：username - 用户名
 *       password - 密码
 * 返回值：成功返回true，失败返回false
 */
bool UserManager::registerUser(const std::string& username, const std::string& password) {
    
    // 使用预处理语句检查用户是否已存在
    std::string checkQuery = "SELECT COUNT(*) FROM users WHERE username = ?";
    std::vector<std::string> checkParams = {username};
    MYSQL_RES* result = DatabaseManager::getInstance().executePreparedQuery(checkQuery, checkParams);
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && atoi(row[0]) > 0) {
            // 用户已存在
            mysql_free_result(result);
            return false;
        }
        mysql_free_result(result);
    }
    
    // 使用预处理语句插入新用户
    std::string insertQuery = "INSERT INTO users (username, password) VALUES (?, ?)";
    std::vector<std::string> insertParams = {username, password};
    int rowsAffected = DatabaseManager::getInstance().executePreparedUpdate(insertQuery, insertParams);
    return rowsAffected > 0;
}

/**
 * 用户登录
 * 功能：验证用户登录信息
 * 参数：username - 用户名
 *       password - 密码
 * 返回值：成功返回true，失败返回false
 */
bool UserManager::loginUser(const std::string& username, const std::string& password) {
    
    // 使用预处理语句验证用户名和密码
    std::string checkQuery = "SELECT COUNT(*) FROM users WHERE username = ? AND password = ?";
    std::vector<std::string> checkParams = {username, password};
    MYSQL_RES* result = DatabaseManager::getInstance().executePreparedQuery(checkQuery, checkParams);
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && atoi(row[0]) > 0) {
            // 用户名和密码匹配
            mysql_free_result(result);
            return true;
        }
        mysql_free_result(result);
    }
    // 无效的用户名或密码
    return false;
}

/**
 * 检查用户是否存在
 * 功能：检查指定用户名是否存在
 * 参数：username - 用户名
 * 返回值：存在返回true，不存在返回false
 */
bool UserManager::userExists(const std::string& username) {
    
    // 使用预处理语句检查用户是否存在
    std::string checkQuery = "SELECT COUNT(*) FROM users WHERE username = ?";
    std::vector<std::string> checkParams = {username};
    MYSQL_RES* result = DatabaseManager::getInstance().executePreparedQuery(checkQuery, checkParams);
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        bool exists = row && atoi(row[0]) > 0;
        mysql_free_result(result);
        return exists;
    }
    return false;
}

/**
 * 获取单例实例
 * 功能：获取用户管理器的单例实例
 * 返回值：UserManager的引用
 */
UserManager& UserManager::getInstance() {
    static UserManager instance;
    return instance;
}