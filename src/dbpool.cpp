#include "dbpool.h"
#include <iostream>
#include "config.h"

DatabasePool::DatabasePool() {
    currentConnections = 0;
    isInitialized = false;
}

DatabasePool::~DatabasePool() {
    closePool();
}

bool DatabasePool::initialize(const std::string& host, const std::string& user, 
                            const std::string& password, const std::string& database, 
                            int maxConn, int minConn, int timeout) {
    if (isInitialized) {
        return true;
    }
    
    this->host = host;
    this->user = user;
    this->password = password;
    this->database = database;
    
    // 从配置文件读取连接池参数
    this->maxConnections = ConfigManager::getInstance().getInt("db_pool_max_connections", maxConn);
    this->minConnections = ConfigManager::getInstance().getInt("db_pool_min_connections", minConn);
    this->connectionTimeout = ConfigManager::getInstance().getInt("db_pool_connection_timeout", timeout);
    
    for (int i = 0; i < minConnections; i++) {
        MYSQL* conn = createConnection();
        if (conn) {
            connectionQueue.push(conn);
            currentConnections++;
        } else {
            std::cerr << "Failed to create initial database connection" << std::endl;
            closePool();
            return false;
        }
    }
    
    isInitialized = true;
    std::cout << "Database pool initialized successfully. Connections: " << currentConnections << std::endl;
    return true;
}

MYSQL* DatabasePool::createConnection() {
    MYSQL* conn = mysql_init(nullptr);
    if (!conn) {
        std::cerr << "mysql_init failed" << std::endl;
        return nullptr;
    }
    
    bool reconnect = true;
    mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &connectionTimeout);
    
    if (!mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(), 
                           database.c_str(), 0, nullptr, 0)) {
        std::cerr << "mysql_real_connect failed: " << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return nullptr;
    }
    
    if (mysql_set_character_set(conn, "utf8mb4")) {
        std::cerr << "mysql_set_character_set failed: " << mysql_error(conn) << std::endl;
    }
    
    return conn;
}

void DatabasePool::destroyConnection(MYSQL* conn) {
    if (conn) {
        mysql_close(conn);
        currentConnections--;
    }
}

bool DatabasePool::isValidConnection(MYSQL* conn) {
    if (!conn) {
        return false;
    }
    
    return mysql_ping(conn) == 0;
}

MYSQL* DatabasePool::getConnection(int timeout) {
    if (!isInitialized) {
        std::cerr << "Database pool not initialized" << std::endl;
        return nullptr;
    }
    
    std::unique_lock<std::mutex> lock(poolMutex);
    
    auto start = std::chrono::steady_clock::now();
    auto end = start + std::chrono::seconds(timeout);
    
    while (connectionQueue.empty() && currentConnections >= maxConnections) {
        if (cv.wait_until(lock, end) == std::cv_status::timeout) {
            std::cerr << "Get connection timeout" << std::endl;
            return nullptr;
        }
    }
    
    MYSQL* conn = nullptr;
    
    if (!connectionQueue.empty()) {
        conn = connectionQueue.front();
        connectionQueue.pop();
        
        if (!isValidConnection(conn)) {
            std::cout << "Connection invalid, creating new one" << std::endl;
            destroyConnection(conn);
            conn = createConnection();
        }
    } else if (currentConnections < maxConnections) {
        conn = createConnection();
        if (conn) {
            currentConnections++;
            std::cout << "Created new connection, current: " << currentConnections << std::endl;
        }
    }
    
    return conn;
}

void DatabasePool::releaseConnection(MYSQL* conn) {
    if (!conn) {
        return;
    }
    
    std::unique_lock<std::mutex> lock(poolMutex);
    
    if (isValidConnection(conn)) {
        connectionQueue.push(conn);
        cv.notify_one();
    } else {
        destroyConnection(conn);
    }
}

void DatabasePool::getPoolStatus(int& current, int& available, int& max) {
    std::lock_guard<std::mutex> lock(poolMutex);
    current = currentConnections;
    available = connectionQueue.size();
    max = maxConnections;
}

void DatabasePool::closePool() {
    std::lock_guard<std::mutex> lock(poolMutex);
    
    if (!isInitialized) {
        return;
    }
    
    while (!connectionQueue.empty()) {
        MYSQL* conn = connectionQueue.front();
        connectionQueue.pop();
        destroyConnection(conn);
    }
    
    isInitialized = false;
    std::cout << "Database pool closed. Connections destroyed: " << currentConnections << std::endl;
}

DatabasePool& DatabasePool::getInstance() {
    static DatabasePool instance;
    return instance;
}