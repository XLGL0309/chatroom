#include <iostream>
#include <thread>
#include "include/network.h"

int main() {
    // 设置Windows控制台输出编码为UTF-8
    #ifdef _WIN32
    SetConsoleOutputCP(65001);
    #endif
    
    // 初始化网络
    initializeNetwork();
    
    // 创建服务器Socket
    SOCKET serverSocket = createServerSocket(8888);
    
    // 主循环
    while (true) {
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
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}