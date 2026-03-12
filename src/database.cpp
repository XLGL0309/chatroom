#include "../include/database.h"
#include <iostream>

DatabaseManager::DatabaseManager() : connection(nullptr) {
    // 初始化 MySQL 库
    mysql_library_init(0, nullptr, nullptr);
}

DatabaseManager::~DatabaseManager() {
    if (connection) {
        mysql_close(connection);
    }
    // 清理 MySQL 库
    mysql_library_end();
}

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
    
    std::cout << "Database connection successful" << std::endl;
    return true;
}

MYSQL_RES* DatabaseManager::executeQuery(const std::string& query) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if (!isConnected()) {
        std::cerr << "Database not connected" << std::endl;
        return nullptr;
    }
    
    if (mysql_query(connection, query.c_str())) {
        std::cerr << "Query execution failed: " << mysql_error(connection) << std::endl;
        return nullptr;
    }
    
    return mysql_store_result(connection);
}

int DatabaseManager::executeUpdate(const std::string& query) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if (!isConnected()) {
        std::cerr << "Database not connected" << std::endl;
        return -1;
    }
    
    if (mysql_query(connection, query.c_str())) {
        std::cerr << "Update execution failed: " << mysql_error(connection) << std::endl;
        return -1;
    }
    
    return mysql_affected_rows(connection);
}

bool DatabaseManager::isConnected() const {
    return connection != nullptr;
}

// 全局数据库管理器实例
DatabaseManager g_databaseManager;