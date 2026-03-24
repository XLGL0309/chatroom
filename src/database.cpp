/*
 * database.cpp
 * 数据库管理功能的实现文件
 * 功能：实现数据库管理器的方法，用于管理数据库连接和执行SQL查询
 */

#include "../include/database.h"
#include <iostream>

/**
 * 构造函数
 * 功能：初始化数据库管理器并初始化MySQL库
 */
DatabaseManager::DatabaseManager() : connection(nullptr) {
    // 初始化 MySQL 库
    mysql_library_init(0, nullptr, nullptr);
}

/**
 * 析构函数
 * 功能：关闭数据库连接并清理MySQL库
 */
DatabaseManager::~DatabaseManager() {
    if (connection) {
        mysql_close(connection);
    }
    // 清理 MySQL 库
    mysql_library_end();
}

/**
 * 初始化数据库连接
 * 功能：建立与数据库的连接
 * 参数：host - 数据库主机地址
 *       user - 数据库用户名
 *       password - 数据库密码
 *       database - 数据库名称
 * 返回值：成功返回true，失败返回false
 */
bool DatabaseManager::initialize(const std::string& host, const std::string& user, const std::string& password, const std::string& database) {
    // 创建 MySQL 连接
    connection = mysql_init(nullptr);
    if (!connection) {
        std::cerr << "MySQL initialization failed" << std::endl;
        return false;
    }
    
    // 连接到数据库
    if (!mysql_real_connect(connection, host.c_str(), user.c_str(), password.c_str(), database.c_str(), 0, nullptr, 0)) {
        std::cerr << "Database connection failed: " << mysql_error(connection) << std::endl;
        mysql_close(connection);
        connection = nullptr;
        return false;
    }
    
    // 设置字符集为 UTF-8
    mysql_set_character_set(connection, "utf8mb4");
    
    std::cout << "Database connected successfully" << std::endl;
    return true;
}

/**
 * 执行参数化查询
 * 功能：执行带参数的SQL查询
 * 参数：query - 带参数占位符的SQL查询语句
 *       params - 参数值列表
 * 返回值：查询结果集
 */
MYSQL_RES* DatabaseManager::executePreparedQuery(const std::string& query, const std::vector<std::string>& params) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if (!isConnected()) {
        std::cerr << "Database not connected" << std::endl;
        return nullptr;
    }
    
    // 对于简单的计数查询，我们使用普通查询来保持兼容性
    // 因为预处理语句的结果集处理比较复杂
    // 这里我们直接使用字符串拼接，但会对输入进行转义，防止SQL注入
    std::string safeQuery = query;
    size_t pos = 0;
    for (const auto& param : params) {
        pos = safeQuery.find('?', pos);
        if (pos == std::string::npos) break;
        
        // 对参数进行转义，防止SQL注入
        char* escaped = new char[param.length() * 2 + 1];
        mysql_real_escape_string(connection, escaped, param.c_str(), param.length());
        std::string safeParam = escaped;
        delete[] escaped;
        
        safeQuery.replace(pos, 1, "'" + safeParam + "'");
        pos += safeParam.length() + 2;
    }
    
    // 执行普通查询
    if (mysql_query(connection, safeQuery.c_str())) {
        std::cerr << "Query execution failed: " << mysql_error(connection) << std::endl;
        return nullptr;
    }
    
    return mysql_store_result(connection);
}

/**
 * 执行参数化更新
 * 功能：执行带参数的SQL更新语句
 * 参数：query - 带参数占位符的SQL更新语句
 *       params - 参数值列表
 * 返回值：影响的行数
 */
int DatabaseManager::executePreparedUpdate(const std::string& query, const std::vector<std::string>& params) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if (!isConnected()) {
        std::cerr << "Database not connected" << std::endl;
        return -1;
    }
    
    // 对于更新操作，我们使用字符串转义的方式处理参数，以确保操作能够正确执行
    std::string safeQuery = query;
    size_t pos = 0;
    for (const auto& param : params) {
        pos = safeQuery.find('?', pos);
        if (pos == std::string::npos) break;
        
        // 对参数进行转义，防止SQL注入
        char* escaped = new char[param.length() * 2 + 1];
        mysql_real_escape_string(connection, escaped, param.c_str(), param.length());
        std::string safeParam = escaped;
        delete[] escaped;
        
        safeQuery.replace(pos, 1, "'" + safeParam + "'");
        pos += safeParam.length() + 2;
    }
    
    // 执行普通更新
    if (mysql_query(connection, safeQuery.c_str())) {
        std::cerr << "Update execution failed: " << mysql_error(connection) << std::endl;
        return -1;
    }
    
    return mysql_affected_rows(connection);
}

/**
 * 检查数据库连接是否有效
 * 功能：检查当前数据库连接状态
 * 返回值：连接有效返回true，否则返回false
 */
bool DatabaseManager::isConnected() const {
    return connection != nullptr;
}

/**
 * 获取单例实例
 * 功能：获取数据库管理器的单例实例
 * 返回值：DatabaseManager的引用
 */
DatabaseManager& DatabaseManager::getInstance() {
    static DatabaseManager instance;
    return instance;
}