#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <mutex>
#include "include/network.h"
#include "include/database.h"
// 用于密码输入的头文件
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// 客户端连接信息结构体
struct ClientConnection {
    SOCKET socket;
    std::string ip;
};

// 客户端连接列表和互斥锁
std::vector<ClientConnection> clientConnections;
std::mutex clientConnectionsMutex;

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
    if (!g_databaseManager.initialize("localhost", "root", dbPassword, "chatroom")) {
        std::cerr << "Database initialization failed, server cannot start" << std::endl;
        return 1;
    }
    
    // 创建服务器Socket
    SOCKET serverSocket = createServerSocket(8888);
    g_serverSocket = serverSocket; // 设置全局服务器套接字
    
    // 启动控制台输入监听线程
    std::thread inputThread(consoleInputThread);
    inputThread.detach();
    
    // 主循环
    while (g_running) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(serverSocket, &readSet);
        
        // 将所有客户端套接字添加到readSet
        int maxSocket = serverSocket;
        {   // 加锁保护客户端连接列表
            std::lock_guard<std::mutex> lock(clientConnectionsMutex);
            for (const ClientConnection& conn : clientConnections) {
                FD_SET(conn.socket, &readSet);
                if (conn.socket > maxSocket) {
                    maxSocket = conn.socket;
                }
            }
        }
        
        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
#ifdef _WIN32
        int result = select(0, &readSet, nullptr, nullptr, &timeout);
#else
        int result = select(maxSocket + 1, &readSet, nullptr, nullptr, &timeout);
#endif
        
        if (result == 0) continue;
        if (result < 0) break;
        
        // 检查服务器套接字是否有新连接
        if (FD_ISSET(serverSocket, &readSet)) {
            sockaddr_in clientAddr;
            int clientAddrSize = sizeof(clientAddr);
            SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
            if (clientSocket != INVALID_SOCKET) {
                std::string clientIP = inet_ntoa(clientAddr.sin_addr);
                std::cout << "New connection from " << clientIP << std::endl;
                
                // 将新客户端添加到列表
                {   // 加锁保护客户端连接列表
                    std::lock_guard<std::mutex> lock(clientConnectionsMutex);
                    clientConnections.push_back({clientSocket, clientIP});
                }
            }
        }
        
        // 检查客户端套接字是否有数据
        std::vector<SOCKET> socketsToRemove;
        {   // 加锁保护客户端连接列表
            std::lock_guard<std::mutex> lock(clientConnectionsMutex);
            for (const ClientConnection& conn : clientConnections) {
                if (FD_ISSET(conn.socket, &readSet)) {
                    // 处理客户端请求
                    handleClientConnection(conn.socket, conn.ip);
                    // 处理完后移除客户端
                    socketsToRemove.push_back(conn.socket);
                }
            }
            
            // 移除断开连接的客户端
            for (SOCKET socketToRemove : socketsToRemove) {
                auto it = std::find_if(clientConnections.begin(), clientConnections.end(),
                    [socketToRemove](const ClientConnection& conn) { return conn.socket == socketToRemove; });
                if (it != clientConnections.end()) {
                    // 不需要再次关闭，因为 handleClientConnection 已经关闭
                    clientConnections.erase(it);
                    std::cout << "Client disconnected" << std::endl;
                }
            }
        }
    }

    // 关闭所有客户端连接
    {   // 加锁保护客户端连接列表
        std::lock_guard<std::mutex> lock(clientConnectionsMutex);
        for (const ClientConnection& conn : clientConnections) {
            closesocket(conn.socket);
        }
        clientConnections.clear();
    }

    // 关闭服务器套接字
    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
        g_serverSocket = INVALID_SOCKET;
    }
    
#ifdef _WIN32
    WSACleanup();
#endif
    std::cout << "Server closed" << std::endl;
    return 0;
}