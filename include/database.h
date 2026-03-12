#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <mutex>
#include <mysql.h>

class DatabaseManager {
private:
    MYSQL* connection;
    std::mutex dbMutex;

public:
    DatabaseManager();
    ~DatabaseManager();
    
    // 初始化数据库连接
    bool initialize(const std::string& host, const std::string& user, const std::string& password, const std::string& database);
    
    // 执行SQL查询
    MYSQL_RES* executeQuery(const std::string& query);
    
    // 执行SQL更新（INSERT、UPDATE、DELETE）
    int executeUpdate(const std::string& query);
    
    // 检查数据库连接是否有效
    bool isConnected() const;
};

// 全局数据库管理器实例
extern DatabaseManager g_databaseManager;

#endif // DATABASE_H