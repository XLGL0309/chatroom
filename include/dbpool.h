#ifndef DBPOOL_H
#define DBPOOL_H

#include <string>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <mysql.h>
#include <chrono>

class DatabasePool {
private:
    std::queue<MYSQL*> connectionQueue;
    std::string host;
    std::string user;
    std::string password;
    std::string database;
    int maxConnections;
    int minConnections;
    int connectionTimeout;
    int currentConnections;
    std::mutex poolMutex;
    std::condition_variable cv;
    bool isInitialized;

    DatabasePool();
    ~DatabasePool();
    DatabasePool(const DatabasePool&) = delete;
    DatabasePool& operator=(const DatabasePool&) = delete;

    MYSQL* createConnection();
    void destroyConnection(MYSQL* conn);
    bool isValidConnection(MYSQL* conn);

public:
    bool initialize(const std::string& host, const std::string& user, const std::string& password, const std::string& database, 
                   int maxConn = 10, int minConn = 3, int timeout = 30);
    MYSQL* getConnection(int timeout = 5);
    void releaseConnection(MYSQL* conn);
    void getPoolStatus(int& current, int& available, int& max);
    void closePool();
    static DatabasePool& getInstance();
};

#endif // DBPOOL_H