#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <mutex>
#include "include/network.h"
#include "include/database.h"
#include "include/threadpool.h"
// 用于密码输入的头文件
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <errno.h>
#include <string.h>
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
    
    // 启动线程池
    g_threadPool.start();
    
    // 启动控制台输入监听线程
    std::thread inputThread(consoleInputThread);
    inputThread.detach();
    
    // 主循环 - 只处理新连接
    while (g_running) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(serverSocket, &readSet);
        
        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
#ifdef _WIN32
        int result = select(0, &readSet, nullptr, nullptr, &timeout);
        if (result < 0) {
            int error = WSAGetLastError();
            std::cerr << "Select failed: " << error << std::endl;
            continue;
        }
#else
        int result = select(serverSocket + 1, &readSet, nullptr, nullptr, &timeout);
        if (result < 0) {
            std::cerr << "Select failed: " << strerror(errno) << std::endl;
            continue;
        }
#endif
        
        if (result == 0) continue;
        
        // 检查服务器套接字是否有新连接
        if (FD_ISSET(serverSocket, &readSet)) {
            sockaddr_in clientAddr;
            int clientAddrSize = sizeof(clientAddr);
            SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
            if (clientSocket != INVALID_SOCKET) {
                std::string clientIP = inet_ntoa(clientAddr.sin_addr);
                std::cout << "New connection from " << clientIP << std::endl;
                
                // 将新客户端添加到线程池
                g_threadPool.addTask(clientSocket, clientIP);
            } else {
                // 处理accept失败的情况
#ifdef _WIN32
                int error = WSAGetLastError();
                std::cerr << "Accept failed: " << error << std::endl;
#else
                std::cerr << "Accept failed: " << strerror(errno) << std::endl;
#endif
            }
        }
    }

    // 停止线程池
    g_threadPool.stop();

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