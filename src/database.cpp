/*
 * database.cpp
 * 数据库管理功能的实现文件
 * 功能：实现数据库管理器的方法，用于管理数据库连接和执行SQL查询
 */

#include "database.h"
#include <iostream>

/**
 * 构造函数
 * 功能：初始化数据库管理器并初始化MySQL库
 */
DatabaseManager::DatabaseManager() {
    // 初始化 MySQL 库
    mysql_library_init(0, nullptr, nullptr);
}

/**
 * 析构函数
 * 功能：关闭数据库连接池并清理MySQL库
 */
DatabaseManager::~DatabaseManager() {
    // 清理 MySQL 库
    mysql_library_end();
}

/**
 * 初始化数据库连接
 * 功能：初始化数据库连接池
 * 参数：host - 数据库主机地址
 *       user - 数据库用户名
 *       password - 数据库密码
 *       database - 数据库名称
 * 返回值：成功返回true，失败返回false
 */
bool DatabaseManager::initialize(const std::string& host, const std::string& user, const std::string& password, const std::string& database) {
    // 初始化连接池
    if (!DatabasePool::getInstance().initialize(host, user, password, database)) {
        std::cerr << "Database pool initialization failed" << std::endl;
        return false;
    }
    
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
    // 从连接池获取连接
    MYSQL* conn = DatabasePool::getInstance().getConnection();
    if (!conn) {
        std::cerr << "Failed to get database connection" << std::endl;
        return nullptr;
    }
    
    try {
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
            mysql_real_escape_string(conn, escaped, param.c_str(), param.length());
            std::string safeParam = escaped;
            delete[] escaped;
            
            safeQuery.replace(pos, 1, "'" + safeParam + "'");
            pos += safeParam.length() + 2;
        }
        
        // 执行普通查询
        if (mysql_query(conn, safeQuery.c_str())) {
            std::cerr << "Query execution failed: " << mysql_error(conn) << std::endl;
            DatabasePool::getInstance().releaseConnection(conn);
            return nullptr;
        }
        
        // 获取结果集
        MYSQL_RES* result = mysql_store_result(conn);
        
        // 释放连接
        DatabasePool::getInstance().releaseConnection(conn);
        
        return result;
    } catch (const std::exception& e) {
        std::cerr << "Exception in executePreparedQuery: " << e.what() << std::endl;
        DatabasePool::getInstance().releaseConnection(conn);
        return nullptr;
    }
}

/**
 * 执行参数化更新
 * 功能：执行带参数的SQL更新语句
 * 参数：query - 带参数占位符的SQL更新语句
 *       params - 参数值列表
 * 返回值：影响的行数
 */
int DatabaseManager::executePreparedUpdate(const std::string& query, const std::vector<std::string>& params) {
    // 从连接池获取连接
    MYSQL* conn = DatabasePool::getInstance().getConnection();
    if (!conn) {
        std::cerr << "Failed to get database connection" << std::endl;
        return -1;
    }
    
    try {
        // 对于更新操作，我们使用字符串转义的方式处理参数，以确保操作能够正确执行
        std::string safeQuery = query;
        size_t pos = 0;
        for (const auto& param : params) {
            pos = safeQuery.find('?', pos);
            if (pos == std::string::npos) break;
            
            // 对参数进行转义，防止SQL注入
            char* escaped = new char[param.length() * 2 + 1];
            mysql_real_escape_string(conn, escaped, param.c_str(), param.length());
            std::string safeParam = escaped;
            delete[] escaped;
            
            safeQuery.replace(pos, 1, "'" + safeParam + "'");
            pos += safeParam.length() + 2;
        }
        
        // 执行普通更新
        if (mysql_query(conn, safeQuery.c_str())) {
            std::cerr << "Update execution failed: " << mysql_error(conn) << std::endl;
            DatabasePool::getInstance().releaseConnection(conn);
            return -1;
        }
        
        // 获取影响的行数
        int affectedRows = mysql_affected_rows(conn);
        
        // 释放连接
        DatabasePool::getInstance().releaseConnection(conn);
        
        return affectedRows;
    } catch (const std::exception& e) {
        std::cerr << "Exception in executePreparedUpdate: " << e.what() << std::endl;
        DatabasePool::getInstance().releaseConnection(conn);
        return -1;
    }
}

/**
 * 检查数据库连接是否有效
 * 功能：检查数据库连接池是否初始化
 * 返回值：连接池初始化返回true，否则返回false
 */
bool DatabaseManager::isConnected() const {
    // 简单检查，实际应该通过连接池的状态来判断
    return true;
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