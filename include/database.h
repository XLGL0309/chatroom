#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <mutex>
#include <mysql.h>
#include <vector>

class DatabaseManager {
private:
    MYSQL* connection;
    std::mutex dbMutex;
    
    // 私有构造函数，防止外部实例化
    DatabaseManager();
    ~DatabaseManager();
    
    // 禁止复制和赋值
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

public:
    // 初始化数据库连接
    bool initialize(const std::string& host, const std::string& user, const std::string& password, const std::string& database);
    
    // 执行SQL查询
    MYSQL_RES* executeQuery(const std::string& query);
    
    // 执行参数化查询
    MYSQL_RES* executePreparedQuery(const std::string& query, const std::vector<std::string>& params);
    
    // 执行参数化更新
    int executePreparedUpdate(const std::string& query, const std::vector<std::string>& params);
    
    // 检查数据库连接是否有效
    bool isConnected() const;
    
    // 静态方法获取单例实例
    static DatabaseManager& getInstance();
};

#endif // DATABASE_H