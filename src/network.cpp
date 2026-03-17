#include "../include/network.h"
#include "../include/web.h"
#include <iostream>
#include <thread>
#ifdef __linux__
#include <fcntl.h>
#else
#include <ws2tcpip.h> // 为Windows提供inet_ntop
#endif

std::atomic<bool> g_running(true);
SOCKET g_serverSocket = INVALID_SOCKET;

#ifdef __linux__
int g_epoll_fd = -1; // 定义全局epoll实例
std::unordered_map<SOCKET, std::string> g_clientIPMap; // 定义客户端IP映射
std::mutex g_clientIPMapMutex; // 定义客户端IP映射的互斥锁
#endif

// 辅助函数：清理客户端socket（从epoll移除并关闭）
static void cleanupClientSocket(SOCKET clientSocket) {
#ifdef __linux__
    if (g_epoll_fd != -1) {
        if (epoll_ctl(g_epoll_fd, EPOLL_CTL_DEL, clientSocket, nullptr) == -1) {
            std::cerr << "epoll_ctl del failed: " << strerror(errno) << std::endl;
        }
    }
#endif
    closesocket(clientSocket);
}

void initializeNetwork() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup initialization failed" << std::endl;
        exit(1);
    }
#endif
}

SOCKET createServerSocket(int port) {
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
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
        std::cerr << "Port binding failed" << std::endl;
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        exit(1);
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        std::cerr << "Listening failed" << std::endl;
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        exit(1);
    }

    g_serverSocket = serverSocket;
    std::cout << "Server started, port: " << port << std::endl;
    // 替换 inet_ntoa 为 inet_ntop
    char ipBuffer[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &serverAddr.sin_addr, ipBuffer, sizeof(ipBuffer));
    std::cout << "Local IP address: " << ipBuffer << std::endl;
    std::cout << "Enter 'exit', 'quit' or 'stop' to stop the server" << std::endl;

    return serverSocket;
}

void handleClientConnection(SOCKET clientSocket, const std::string& clientIP) {
    // 获取客户端IP地址
    std::string actualClientIP = clientIP;
    #ifdef __linux__
    {
        std::lock_guard<std::mutex> lock(g_clientIPMapMutex);
        auto it = g_clientIPMap.find(clientSocket);
        if (it != g_clientIPMap.end()) {
            actualClientIP = it->second;
            g_clientIPMap.erase(it); // 取完就清理，避免内存泄漏
        }
    }
    #endif
    
    char buffer[4096];
    std::string request;
    int bytesRead;
    
    // 循环接收，直到收到完整的 HTTP 请求
    while (true) {
        #ifdef _WIN32
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        #else
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0); // Linux 也支持 recv
        #endif
        
        if (bytesRead <= 0) {
            cleanupClientSocket(clientSocket); // 调用辅助函数
            return;
        }
        
        request.append(buffer, bytesRead);
        
        // 检查是否收到完整的 HTTP 请求头（寻找 \r\n\r\n）
        size_t headerEnd = request.find("\r\n\r\n");
        if (headerEnd != std::string::npos) {
            // 检查是否有 Content-Length 头部
            size_t contentLengthPos = request.find("Content-Length:");
            if (contentLengthPos != std::string::npos && contentLengthPos < headerEnd) {
                // 提取 Content-Length 值
                size_t valueStart = request.find(" ", contentLengthPos) + 1;
                size_t valueEnd = request.find("\r\n", valueStart);
                std::string contentLengthStr = request.substr(valueStart, valueEnd - valueStart);
                int contentLength = std::stoi(contentLengthStr);
                
                // 计算已接收的请求体大小
                size_t bodyStart = headerEnd + 4; // 跳过 \r\n\r\n
                // 如果请求体还没接收完，继续接收
                if (request.length() - bodyStart < contentLength) {
                    continue;
                }
            }
            // 要么没有请求体，要么请求体已接收完整
            break;
        }
    }
    std::string response = handleHttpRequest(request, actualClientIP);

    // 循环发送，直到所有数据发完
    int totalSent = 0;
    int responseLen = response.length();
    while (totalSent < responseLen) {
        int bytesSent = send(clientSocket, response.c_str() + totalSent, responseLen - totalSent, 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Send failed: " <<  
                #ifdef _WIN32
                    WSAGetLastError()
                #else
                    strerror(errno)
                #endif
                << std::endl;
            break;
        }
        totalSent += bytesSent;
    }
    
    cleanupClientSocket(clientSocket); // 调用辅助函数
}