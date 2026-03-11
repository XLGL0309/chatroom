#include <iostream>
#include <thread>
#include <string>
#include "include/network.h"

void consoleInputThread() {
    std::string input;
    while (g_running) {
        std::getline(std::cin, input);
        if (input == "exit" || input == "quit" || input == "stop") {
            std::cout << "正在关闭服务器..." << std::endl;
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
    // 设置Windows控制台输出编码为UTF-8
    #ifdef _WIN32
    SetConsoleOutputCP(65001);
    #endif
    
    // 初始化网络
    initializeNetwork();
    
    // 创建服务器Socket
    SOCKET serverSocket = createServerSocket(8888);
    
    // 启动控制台输入监听线程
    std::thread inputThread(consoleInputThread);
    inputThread.detach();
    
    // 主循环
    while (g_running) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(serverSocket, &readSet);
        
        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
#ifdef _WIN32
        int result = select(0, &readSet, nullptr, nullptr, &timeout);
#else
        int result = select(serverSocket + 1, &readSet, nullptr, nullptr, &timeout);
#endif
        
        if (result == 0) continue;
        if (result < 0) break;
        
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            continue;
        }

        std::string clientIP = inet_ntoa(clientAddr.sin_addr);

        // 创建线程处理客户端请求
        std::thread(handleClientConnection, clientSocket, clientIP).detach();
    }

    closesocket(serverSocket);
    g_serverSocket = INVALID_SOCKET;
#ifdef _WIN32
    WSACleanup();
#endif
    std::cout << "服务器已关闭" << std::endl;
    return 0;
}