#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <atomic>    // 包含atomic
#include <unordered_set> // 包含unordered_set
#include <mutex>     // 包含mutex

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

// 网络管理器类，使用单例模式管理网络相关的全局变量
class NetworkManager {
private:
    // 客户端Socket集合
    std::unordered_set<SOCKET> clientSocketSet;
    std::mutex clientSocketSetMutex;
    
    // 服务运行状态
    std::atomic<bool> running;
    
    // 服务器套接字
    SOCKET serverSocket;
    
    // Linux平台的epoll实例
    #ifdef __linux__
    int epoll_fd;
    #endif
    
    // 私有构造函数，防止外部实例化
    NetworkManager() : running(true), serverSocket(INVALID_SOCKET) {
        #ifdef __linux__
        epoll_fd = -1;
        #endif
    }
    ~NetworkManager() {}
    
    // 禁止复制和赋值
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

public:
    // 静态方法获取单例实例
    static NetworkManager& getInstance();
    
    // 客户端Socket集合相关方法
    std::unordered_set<SOCKET>& getClientSocketSet();
    std::mutex& getClientSocketSetMutex();
    
    // 服务运行状态相关方法
    std::atomic<bool>& getRunning();
    void setRunning(bool value);
    
    // 服务器套接字相关方法
    SOCKET& getServerSocket();
    void setServerSocket(SOCKET socket);
    
    // epoll实例相关方法（仅Linux平台）
    #ifdef __linux__
    int getEpollFd();
    void setEpollFd(int fd);
    #endif
};

void initializeNetwork();
SOCKET createServerSocket(int port);
void handleClientConnection(SOCKET clientSocket);

#endif // NETWORK_H