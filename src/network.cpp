#include "../include/network.h"
#include "../include/web.h"
#include <iostream>
#include <thread>

void initializeNetwork() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup初始化失败" << std::endl;
        exit(1);
    }
#endif
}

SOCKET createServerSocket(int port) {
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket创建失败" << std::endl;
#ifdef _WIN32
        WSACleanup();
#endif
        exit(1);
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "端口绑定失败" << std::endl;
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        exit(1);
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        std::cerr << "监听失败" << std::endl;
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        exit(1);
    }

    std::cout << "服务器已启动，端口: " << port << std::endl;
    std::cout << "本地IP地址: " << inet_ntoa(serverAddr.sin_addr) << std::endl;

    return serverSocket;
}

void handleClientConnection(SOCKET clientSocket, const std::string& clientIP) {
    char buffer[4096];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        closesocket(clientSocket);
        return;
    }

    std::string request(buffer, bytesRead);
    std::string response = handleHttpRequest(request, clientIP);

    int bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
    closesocket(clientSocket);
}