/*
 * network.h
 * 网络相关功能的头文件
 * 功能：定义网络管理器类和网络相关函数
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <atomic>    // 包含atomic
#include <unordered_set> // 包含unordered_set
#include <mutex>     // 包含mutex
#include <map>       // 包含map
#include <chrono>    // 包含chrono

// 跨平台Socket支持
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#define WSADATA int
#define WSAStartup(a, b) 0
#define WSACleanup() 0

#endif

// 辅助函数声明
void cleanupClientSocket(SOCKET clientSocket);
std::string receiveHttpRequest(SOCKET clientSocket);
bool sendHttpResponse(SOCKET clientSocket, const std::string& response);
void parseHttpRequest(const std::string& request, std::string& method, std::string& path, std::string& httpVersion, std::string& connectionHeader, std::string& body);

/**
 * 网络管理器类
 * 功能：使用单例模式管理网络相关的全局变量
 * 说明：采用懒汉式单例模式，确保线程安全
 */
class NetworkManager {
private:
    // 客户端Socket集合
    std::unordered_set<SOCKET> clientSocketSet;
    // 客户端Socket集合的互斥锁
    std::mutex clientSocketSetMutex;

    // Socket最后活动时间映射（用于心跳检测）
    std::map<SOCKET, std::chrono::steady_clock::time_point> socketLastActivity;
    // 活动时间映射的互斥锁
    std::mutex socketActivityMutex;

    // 正在处理的Socket集合（防止同一socket被多个线程同时处理）
    std::unordered_set<SOCKET> processingSockets;
    // processingSockets的互斥锁
    std::mutex processingSocketsMutex;
    
    // 服务运行状态
    std::atomic<bool> running;
    
    // 服务器套接字
    SOCKET serverSocket;
    
    // Linux平台的epoll实例
    #ifdef __linux__
    int epoll_fd;
    #endif
    
    /**
     * 私有构造函数
     * 功能：初始化网络管理器
     */
    NetworkManager() : running(true), serverSocket(INVALID_SOCKET) {
        #ifdef __linux__
        epoll_fd = -1;
        #endif
    }
    
    /**
     * 析构函数
     */
    ~NetworkManager() {}
    
    // 禁止复制和赋值
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

public:
    /**
     * 获取单例实例
     * 返回值：NetworkManager的引用
     */
    static NetworkManager& getInstance();
    
    /**
     * 获取客户端Socket集合
     * 返回值：客户端Socket集合的引用
     */
    std::unordered_set<SOCKET>& getClientSocketSet();
    
    /**
     * 获取客户端Socket集合的互斥锁
     * 返回值：互斥锁的引用
     */
    std::mutex& getClientSocketSetMutex();
    
    /**
     * 获取服务运行状态
     * 返回值：运行状态的引用
     */
    std::atomic<bool>& getRunning();
    
    /**
     * 设置服务运行状态
     * 参数：value - 运行状态
     */
    void setRunning(bool value);
    
    /**
     * 获取服务器套接字
     * 返回值：服务器套接字的引用
     */
    SOCKET& getServerSocket();
    
    /**
     * 设置服务器套接字
     * 参数：socket - 服务器套接字
     */
    void setServerSocket(SOCKET socket);

    // Socket活动时间管理方法
    /**
     * 更新Socket最后活动时间
     * 参数：clientSocket - 客户端Socket
     */
    void updateSocketActivity(SOCKET clientSocket);

    /**
     * 获取Socket最后活动时间
     * 参数：clientSocket - 客户端Socket
     * 返回值：最后活动时间点
     */
    std::chrono::steady_clock::time_point getSocketLastActivity(SOCKET clientSocket);

    /**
     * 清理超时Socket
     * 功能：清理超过指定时间没有活动的Socket
     * 参数：timeoutSeconds - 超时时间（秒）
     * 返回值：空
     */
    void cleanupTimeoutSockets(int timeoutSeconds);

    /**
     * 获取socketActivityMutex
     * 返回值：socketActivityMutex的引用
     */
    std::mutex& getSocketActivityMutex();

    /**
     * 获取socketLastActivityMap
     * 返回值：socketLastActivity的引用
     */
    std::map<SOCKET, std::chrono::steady_clock::time_point>& getSocketLastActivityMap();

    /**
     * 检查Socket是否正在被处理
     * 参数：clientSocket - 客户端Socket
     * 返回值：true表示正在处理，false表示不在处理
     */
    bool isProcessing(SOCKET clientSocket);

    /**
     * 标记Socket为正在处理或取消标记
     * 参数：clientSocket - 客户端Socket
     *       processing - true表示标记为正在处理，false表示取消标记
     */
    void markProcessing(SOCKET clientSocket, bool processing);
    
    // epoll实例相关方法（仅Linux平台）
    #ifdef __linux__
    /**
     * 获取epoll文件描述符
     * 返回值：epoll文件描述符
     */
    int getEpollFd();
    
    /**
     * 设置epoll文件描述符
     * 参数：fd - epoll文件描述符
     */
    void setEpollFd(int fd);
    #endif
};

/**
 * 初始化网络
 * 功能：初始化网络库，为Socket操作做准备
 */
void initializeNetwork();

/**
 * 创建服务器Socket
 * 功能：创建并配置服务器Socket
 * 参数：port - 服务器端口
 * 返回值：创建的服务器Socket
 */
SOCKET createServerSocket(int port);

/**
 * 处理客户端连接
 * 功能：处理客户端的请求和响应
 * 参数：clientSocket - 客户端Socket
 */
void handleClientConnection(SOCKET clientSocket);

#endif // NETWORK_H