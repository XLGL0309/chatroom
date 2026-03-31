/*
 * main.cpp
 * 聊天服务器主文件
 * 功能：初始化服务器，处理客户端连接，管理线程池
 */

#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <mutex>
#include "include/network.h"
#include "include/database.h"
#include "include/threadpool.h"
#include "include/config.h"

// 用于密码输入的头文件
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h> // 包含fcntl.h
#endif

/**
 * 控制台输入线程函数
 * 功能：监听用户输入，处理退出命令
 */
void consoleInputThread() {
    std::string input;
    while (NetworkManager::getInstance().getRunning()) {
        std::getline(std::cin, input);
        if (input == "exit" || input == "quit" || input == "stop") {
            std::cout << "Shutting down server..." << std::endl;
            NetworkManager::getInstance().setRunning(false);
            if (NetworkManager::getInstance().getServerSocket() != INVALID_SOCKET) {
                closesocket(NetworkManager::getInstance().getServerSocket());
                NetworkManager::getInstance().setServerSocket(INVALID_SOCKET);
            }
            break;
        }
    }
}

/**
 * 主函数
 * 功能：初始化服务器，处理客户端连接
 * 返回值：0表示成功，1表示失败
 */
int main() {
    // 初始化网络
    initializeNetwork();
    
    // 从配置文件读取数据库配置
    std::string dbHost = ConfigManager::getInstance().get("db_host", "localhost");
    std::string dbUser = ConfigManager::getInstance().get("db_user", "root");
    std::string dbName = ConfigManager::getInstance().get("db_name", "chatroom");
    
    // 输入数据库密码
    std::string dbPassword;
    std::cout << "Please enter database password: ";
    
    // 隐藏密码输入
    #ifdef _WIN32
    // Windows平台
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
    std::getline(std::cin, dbPassword);
    SetConsoleMode(hStdin, mode);
    #else
    // Linux/Unix平台
    char* password = getpass("");
    dbPassword = password;
    #endif
    std::cout << std::endl;
    
    // 初始化数据库连接
    if (!DatabaseManager::getInstance().initialize(dbHost, dbUser, dbPassword, dbName)) {
        std::cerr << "Database initialization failed, server cannot start" << std::endl;
        return 1;
    }
    
    // 从配置文件读取服务器端口
    int serverPort = ConfigManager::getInstance().getInt("server_port", 8888);
    
    // 创建服务器Socket
    SOCKET serverSocket = createServerSocket(serverPort);
    NetworkManager::getInstance().setServerSocket(serverSocket); // 设置服务器套接字
    
    // 设置serverSocket为非阻塞
    #ifdef _WIN32
    u_long nonBlockMode = 1; // 1=非阻塞
    if (ioctlsocket(serverSocket, FIONBIO, &nonBlockMode) == SOCKET_ERROR) {
        std::cerr << "Set serverSocket non-block failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    #endif
    
    // 启动线程池
    ThreadPool::getInstance(MAX_THREADS).start();
    
    // 启动控制台输入监听线程
    std::thread inputThread(consoleInputThread);
    inputThread.detach();
    
    // 主循环 - 处理新连接和客户端通信
    #ifdef _WIN32
    // Windows平台使用select
    while (NetworkManager::getInstance().getRunning()) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(serverSocket, &readSet);
        
        // 将所有客户端套接字添加到readSet
        {  
            std::lock_guard<std::mutex> lock(NetworkManager::getInstance().getClientSocketSetMutex());
            for (SOCKET clientSocket : NetworkManager::getInstance().getClientSocketSet()) {
                FD_SET(clientSocket, &readSet);
            }
        }
        
        // 设置超时时间为1秒
        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        // 调用select检查可读事件
        int result = select(0, &readSet, nullptr, nullptr, &timeout);
        if (result < 0) {
            int error = WSAGetLastError();
            std::cerr << "Select failed: " << error << std::endl;
            continue;
        }
        
        // 超时，继续循环
        if (result == 0) continue;
        
        // 检查服务器套接字是否有新连接
        if (FD_ISSET(serverSocket, &readSet)) {
            // 双重检查：先确认服务仍在运行，再accept
            if (!NetworkManager::getInstance().getRunning()) {
                continue;
            }
            
            // 循环accept，处理所有待处理连接
            while (true) {
                sockaddr_in clientAddr;
                int clientAddrSize = sizeof(clientAddr);
                SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
                if (clientSocket == INVALID_SOCKET) {
                    // 没有更多连接时退出循环
                    int error = WSAGetLastError();
                    if (error == WSAEWOULDBLOCK) {
                        // 正常情况，没有更多连接
                        break;
                    } else if (error != WSAECONNRESET) {
                        // 真正的错误
                        std::cerr << "Accept failed: " << error << std::endl;
                    }
                    break;
                }
                
                // 设置clientSocket为非阻塞模式
                u_long nonBlockMode = 1; // 1=非阻塞
                if (ioctlsocket(clientSocket, FIONBIO, &nonBlockMode) == SOCKET_ERROR) {
                    std::cerr << "Set clientSocket non-block failed: " << WSAGetLastError() << std::endl;
                    closesocket(clientSocket);
                    continue;
                }
                
                // 存储客户端套接字
                {  
                    std::lock_guard<std::mutex> lock(NetworkManager::getInstance().getClientSocketSetMutex());
                    NetworkManager::getInstance().getClientSocketSet().insert(clientSocket);
                }
            }
        }
        
        // 检查客户端套接字是否有数据
        {  
            std::lock_guard<std::mutex> lock(NetworkManager::getInstance().getClientSocketSetMutex());
            for (auto it = NetworkManager::getInstance().getClientSocketSet().begin(); it != NetworkManager::getInstance().getClientSocketSet().end(); ++it) {
                SOCKET clientSocket = *it;
                
                if (FD_ISSET(clientSocket, &readSet)) {
                    // 客户端有数据，交给线程池处理
                    ThreadPool::getInstance().addTask(clientSocket);
                }
            }
        }
        
        // 定期清理超时的socket（心跳检测）
        if (NetworkManager::getInstance().cleanupTimeoutSockets(70) > 0) {
            // 清理了超时的socket
        }
    }
    #else
    // Linux平台使用epoll
    // 设置serverSocket为非阻塞
    int flags = fcntl(serverSocket, F_GETFL, 0);
    if (fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "fcntl failed: " << strerror(errno) << std::endl;
        return 1;
    }
    
    // 创建epoll实例
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        std::cerr << "epoll_create1 failed: " << strerror(errno) << std::endl;
        return 1;
    }
    
    // 将epoll文件描述符保存到NetworkManager
    #ifdef __linux__
    NetworkManager::getInstance().setEpollFd(epoll_fd); // 赋值给NetworkManager，让handleClientConnection能访问
    #endif

    // 注册服务器Socket到epoll
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET; // 边缘触发
    event.data.fd = serverSocket;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverSocket, &event) == -1) {
        std::cerr << "epoll_ctl failed: " << strerror(errno) << std::endl;
        close(epoll_fd);
        #ifdef __linux__
        NetworkManager::getInstance().setEpollFd(-1);
        #endif
        return 1;
    }

    const int MAX_EVENTS = 1024; // 增大事件数组大小，支持更多并发
    struct epoll_event events[MAX_EVENTS];

    // 主循环
    while (NetworkManager::getInstance().getRunning()) {
        // 等待事件，超时时间1秒
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000);
        
        // 定期清理超时的socket（心跳检测）
        if (NetworkManager::getInstance().cleanupTimeoutSockets(70) > 0) {
            // 清理了超时的socket
        }
        if (num_events == -1) {
            if (errno == EINTR) {
                continue;
            }
            std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
            break;
        }

        // 处理所有事件
        for (int i = 0; i < num_events; i++) {
            // 区分serverSocket和clientSocket
            if (events[i].data.fd == serverSocket) {
                // 情况1：serverSocket的连接事件
                // 双重检查：先确认服务仍在运行，再accept
                if (!NetworkManager::getInstance().getRunning()) {
                    continue;
                }
                
                // 循环accept，处理所有待处理连接（ET模式必须）
                while (true) {
                    sockaddr_in clientAddr;
                    int clientAddrSize = sizeof(clientAddr);
                    SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
                    if (clientSocket == INVALID_SOCKET) {
                        // 没有更多连接时退出循环
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        std::cerr << "Accept failed: " << strerror(errno) << std::endl;
                        break;
                    }
                    
                    // 处理新连接
                    // 存储客户端套接字
                    {
                        std::lock_guard<std::mutex> lock(NetworkManager::getInstance().getClientSocketSetMutex());
                        NetworkManager::getInstance().getClientSocketSet().insert(clientSocket);
                    }
                    
                    // 设置clientSocket为非阻塞
                    int client_flags = fcntl(clientSocket, F_GETFL, 0);
                    if (fcntl(clientSocket, F_SETFL, client_flags | O_NONBLOCK) == -1) {
                        std::cerr << "fcntl client failed: " << strerror(errno) << std::endl;
                        // 清理客户端套接字
                        {
                            std::lock_guard<std::mutex> lock(NetworkManager::getInstance().getClientSocketSetMutex());
                            NetworkManager::getInstance().getClientSocketSet().erase(clientSocket);
                        }
                        closesocket(clientSocket);
                        continue;
                    }
                    
                    // 将新的clientSocket注册到epoll
                    struct epoll_event client_event;
                    client_event.events = EPOLLIN | EPOLLET | EPOLLONESHOT; // 边缘触发 + 一次性事件
                    client_event.data.fd = clientSocket;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clientSocket, &client_event) == -1) {
                        std::cerr << "epoll_ctl add client failed: " << strerror(errno) << std::endl;
                        // 清理客户端套接字
                        {
                            std::lock_guard<std::mutex> lock(NetworkManager::getInstance().getClientSocketSetMutex());
                            NetworkManager::getInstance().getClientSocketSet().erase(clientSocket);
                        }
                        closesocket(clientSocket);
                    }
                }
            } else {
                // 情况2：clientSocket的可读事件
                // 双重检查：先确认服务仍在运行
                if (!NetworkManager::getInstance().getRunning()) {
                    continue;
                }
                
                SOCKET clientSocket = events[i].data.fd;
                // 将clientSocket交给线程池处理
                ThreadPool::getInstance().addTask(clientSocket);
                // 不立即从集合中移除，由线程处理完后根据keepAlive决定
                
            }
        }
    }

    // 清理epoll实例
    close(epoll_fd);
    #ifdef __linux__
    NetworkManager::getInstance().setEpollFd(-1);
    #endif
    #endif

    
    // 清理所有客户端连接
    {
        std::lock_guard<std::mutex> lock(NetworkManager::getInstance().getClientSocketSetMutex());
        auto& clientSockets = NetworkManager::getInstance().getClientSocketSet();
        for (SOCKET clientSocket : clientSockets) {
            closesocket(clientSocket);
        }
        clientSockets.clear();
    }
    
    // 关闭服务器套接字（仅在未被关闭的情况下）
    if (serverSocket != INVALID_SOCKET && NetworkManager::getInstance().getServerSocket() != INVALID_SOCKET) {
        closesocket(serverSocket);
        NetworkManager::getInstance().getServerSocket() = INVALID_SOCKET;
    }
    
    // 清理Windows网络资源
#ifdef _WIN32
    WSACleanup();
#endif
    
    std::cout << "Server closed" << std::endl;
    return 0;
}