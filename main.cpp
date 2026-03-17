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


void consoleInputThread() {
    std::string input;
    while (g_running) {
        std::getline(std::cin, input);
        if (input == "exit" || input == "quit" || input == "stop") {
            std::cout << "Shutting down server..." << std::endl;
            g_running = false;
            if (g_serverSocket != INVALID_SOCKET) {
                closesocket(g_serverSocket);
                g_serverSocket = INVALID_SOCKET;
            }
            break;
        }
    }
}

int main() {
    // 初始化网络
    initializeNetwork();
    
    // 从配置文件读取数据库配置
    std::string dbHost = g_configManager.get("db_host", "localhost");
    std::string dbUser = g_configManager.get("db_user", "root");
    std::string dbName = g_configManager.get("db_name", "chatroom");
    
    // Input database password
    std::string dbPassword;
    std::cout << "Please enter database password: ";
    // Hide password input
    #ifdef _WIN32
    // Windows platform
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
    std::getline(std::cin, dbPassword);
    SetConsoleMode(hStdin, mode);
    #else
    // Linux/Unix platform
    char* password = getpass("");
    dbPassword = password;
    #endif
    std::cout << std::endl;
    
    // Initialize database connection
    if (!g_databaseManager.initialize(dbHost, dbUser, dbPassword, dbName)) {
        std::cerr << "Database initialization failed, server cannot start" << std::endl;
        return 1;
    }
    
    // 从配置文件读取服务器端口
    int serverPort = g_configManager.getInt("server_port", 8888);
    
    // 创建服务器Socket
    SOCKET serverSocket = createServerSocket(serverPort);
    g_serverSocket = serverSocket; // 设置全局服务器套接字
    
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
    g_threadPool.start();
    
    // 启动控制台输入监听线程
    std::thread inputThread(consoleInputThread);
    inputThread.detach();
    
    // 主循环 - 只处理新连接
    #ifdef _WIN32
    // Windows平台使用select
    while (g_running) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(serverSocket, &readSet);
        
        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int result = select(0, &readSet, nullptr, nullptr, &timeout);
        if (result < 0) {
            int error = WSAGetLastError();
            std::cerr << "Select failed: " << error << std::endl;
            continue;
        }
        
        if (result == 0) continue;
        
        // 检查服务器套接字是否有新连接
        if (FD_ISSET(serverSocket, &readSet)) {
            // 双重检查：先确认服务仍在运行，再accept
                if (!g_running) {
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
                    
                    // accept拿到clientSocket后，立即把它改回阻塞模式
                    u_long blockMode = 0; // 0=阻塞模式
                    if (ioctlsocket(clientSocket, FIONBIO, &blockMode) == SOCKET_ERROR) {
                        std::cerr << "Set clientSocket block failed: " << WSAGetLastError() << std::endl;
                        closesocket(clientSocket);
                        continue;
                    }
                    
                    std::string clientIP = inet_ntoa(clientAddr.sin_addr);
                    
                    // 将新客户端添加到线程池
                    g_threadPool.addTask(clientSocket, clientIP);
                }
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
    
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        std::cerr << "epoll_create1 failed: " << strerror(errno) << std::endl;
        return 1;
    }
    g_epoll_fd = epoll_fd; // 赋值给全局变量，让handleClientConnection能访问

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET; // 边缘触发
    event.data.fd = serverSocket;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverSocket, &event) == -1) {
        std::cerr << "epoll_ctl failed: " << strerror(errno) << std::endl;
        close(epoll_fd);
        g_epoll_fd = -1;
        return 1;
    }

    const int MAX_EVENTS = 1024; // 增大事件数组大小，支持更多并发
    struct epoll_event events[MAX_EVENTS];

    while (g_running) {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000); // 1秒超时
        if (num_events == -1) {
            if (errno == EINTR) {
                continue;
            }
            std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
            break;
        }

        for (int i = 0; i < num_events; i++) {
            // 区分serverSocket和clientSocket
            if (events[i].data.fd == serverSocket) {
                // 情况1：serverSocket的连接事件
                // 双重检查：先确认服务仍在运行，再accept
                if (!g_running) {
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
                    char ipBuffer[INET_ADDRSTRLEN] = {0};
                    inet_ntop(AF_INET, &clientAddr.sin_addr, ipBuffer, sizeof(ipBuffer));
                    std::string clientIP = ipBuffer;
                    
                    // 存fd->ip的映射
                    {
                        std::lock_guard<std::mutex> lock(g_clientIPMapMutex);
                        g_clientIPMap[clientSocket] = clientIP;
                    }
                    
                    // 设置clientSocket为非阻塞
                    int client_flags = fcntl(clientSocket, F_GETFL, 0);
                    if (fcntl(clientSocket, F_SETFL, client_flags | O_NONBLOCK) == -1) {
                        std::cerr << "fcntl client failed: " << strerror(errno) << std::endl;
                        // 清理IP映射
                        {
                            std::lock_guard<std::mutex> lock(g_clientIPMapMutex);
                            g_clientIPMap.erase(clientSocket);
                        }
                        closesocket(clientSocket);
                        continue;
                    }
                    
                    // 将新的clientSocket注册到epoll
                    struct epoll_event client_event;
                    client_event.events = EPOLLIN | EPOLLET; // 边缘触发
                    client_event.data.fd = clientSocket;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clientSocket, &client_event) == -1) {
                        std::cerr << "epoll_ctl add client failed: " << strerror(errno) << std::endl;
                        // 清理IP映射
                        {
                            std::lock_guard<std::mutex> lock(g_clientIPMapMutex);
                            g_clientIPMap.erase(clientSocket);
                        }
                        closesocket(clientSocket);
                    }
                }
            } else {
                // 情况2：clientSocket的可读事件
                // 双重检查：先确认服务仍在运行
                if (!g_running) {
                    continue;
                }
                
                SOCKET clientSocket = events[i].data.fd;
                // 将clientSocket交给线程池处理
                g_threadPool.addTask(clientSocket, "");
            }
        }
    }

    // 清理epoll实例
    close(epoll_fd);
    g_epoll_fd = -1;
    #endif

    
    // 关闭服务器套接字（仅在未被关闭的情况下）
    if (serverSocket != INVALID_SOCKET && g_serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
        g_serverSocket = INVALID_SOCKET;
    }
    
#ifdef _WIN32
    WSACleanup();
#endif
    std::cout << "Server closed" << std::endl;
    return 0;
}