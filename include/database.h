/*
 * database.h
 * 数据库管理功能的头文件
 * 功能：定义数据库管理器类，用于管理数据库连接和执行SQL查询
 */

#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <mutex>
#include <mysql.h>
#include <vector>

/**
 * 数据库管理器类
 * 功能：使用单例模式管理数据库连接
 * 说明：采用懒汉式单例模式，确保线程安全
 */
class DatabaseManager {
private:
    // 数据库连接
    MYSQL* connection;
    // 数据库操作的互斥锁
    std::mutex dbMutex;
    
    /**
     * 私有构造函数
     * 功能：初始化数据库管理器
     */
    DatabaseManager();
    
    /**
     * 析构函数
     * 功能：关闭数据库连接
     */
    ~DatabaseManager();
    
    // 禁止复制和赋值
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

public:
    /**
     * 初始化数据库连接
     * 功能：建立与数据库的连接
     * 参数：host - 数据库主机地址
     *       user - 数据库用户名
     *       password - 数据库密码
     *       database - 数据库名称
     * 返回值：成功返回true，失败返回false
     */
    bool initialize(const std::string& host, const std::string& user, const std::string& password, const std::string& database);
    
    /**
     * 执行SQL查询
     * 功能：执行非参数化的SQL查询
     * 参数：query - SQL查询语句
     * 返回值：查询结果集
     */
    MYSQL_RES* executeQuery(const std::string& query);
    
    /**
     * 执行参数化查询
     * 功能：执行带参数的SQL查询
     * 参数：query - 带参数占位符的SQL查询语句
     *       params - 参数值列表
     * 返回值：查询结果集
     */
    MYSQL_RES* executePreparedQuery(const std::string& query, const std::vector<std::string>& params);
    
    /**
     * 执行参数化更新
     * 功能：执行带参数的SQL更新语句
     * 参数：query - 带参数占位符的SQL更新语句
     *       params - 参数值列表
     * 返回值：影响的行数
     */
    int executePreparedUpdate(const std::string& query, const std::vector<std::string>& params);
    
    /**
     * 检查数据库连接是否有效
     * 功能：检查当前数据库连接状态
     * 返回值：连接有效返回true，否则返回false
     */
    bool isConnected() const;
    
    /**
     * 获取单例实例
     * 功能：获取数据库管理器的单例实例
     * 返回值：DatabaseManager的引用
     */
    static DatabaseManager& getInstance();
};

#endif // DATABASE_H