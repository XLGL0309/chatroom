/*
 * network.cpp
 * 网络相关功能的实现文件
 * 功能：实现网络管理器、网络初始化、服务器Socket创建和客户端连接处理
 */

#include "../include/network.h"
#include "../include/web.h"
#include <iostream>
#include <thread>
#ifdef __linux__
#include <fcntl.h>
#else
#include <ws2tcpip.h> // 为Windows提供inet_ntop
#endif

/**
 * 获取NetworkManager的单例实例
 * 返回值：NetworkManager的引用
 */
NetworkManager& NetworkManager::getInstance() {
    static NetworkManager instance;
    return instance;
}

/**
 * 获取客户端Socket集合
 * 返回值：客户端Socket集合的引用
 */
std::unordered_set<SOCKET>& NetworkManager::getClientSocketSet() {
    return clientSocketSet;
}

/**
 * 获取客户端Socket集合的互斥锁
 * 返回值：互斥锁的引用
 */
std::mutex& NetworkManager::getClientSocketSetMutex() {
    return clientSocketSetMutex;
}

/**
 * 获取服务运行状态
 * 返回值：运行状态的引用
 */
std::atomic<bool>& NetworkManager::getRunning() {
    return running;
}

/**
 * 设置服务运行状态
 * 参数：value - 运行状态
 */
void NetworkManager::setRunning(bool value) {
    running = value;
}

/**
 * 获取服务器套接字
 * 返回值：服务器套接字的引用
 */
SOCKET& NetworkManager::getServerSocket() {
    return serverSocket;
}

/**
 * 设置服务器套接字
 * 参数：socket - 服务器套接字
 */
void NetworkManager::setServerSocket(SOCKET socket) {
    serverSocket = socket;
}

#ifdef __linux__
/**
 * 获取epoll文件描述符
 * 返回值：epoll文件描述符
 */
int NetworkManager::getEpollFd() {
    return epoll_fd;
}

/**
 * 设置epoll文件描述符
 * 参数：fd - epoll文件描述符
 */
void NetworkManager::setEpollFd(int fd) {
    epoll_fd = fd;
}
#endif

/**
 * 辅助函数：清理客户端socket
 * 功能：从epoll中移除客户端socket并关闭
 * 参数：clientSocket - 客户端socket
 */
static void cleanupClientSocket(SOCKET clientSocket) {
#ifdef __linux__
    if (NetworkManager::getInstance().getEpollFd() != -1) {
        if (epoll_ctl(NetworkManager::getInstance().getEpollFd(), EPOLL_CTL_DEL, clientSocket, nullptr) == -1) {
            std::cerr << "epoll_ctl del failed: " << strerror(errno) << std::endl;
        }
    }
#endif
    closesocket(clientSocket);
}

/**
 * 辅助函数：接收完整的HTTP请求
 * 功能：从客户端接收完整的HTTP请求
 * 参数：clientSocket - 客户端socket
 * 返回值：完整的HTTP请求字符串
 */
static std::string receiveHttpRequest(SOCKET clientSocket) {
    char buffer[4096];
    std::string request;
    int bytesRead;
    bool headerFound = false;
    int contentLength = -1;
    size_t headerEnd = 0;
    
    while (true) {
        #ifdef _WIN32
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        #else
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0); // Linux 也支持 recv
        #endif
        
        if (bytesRead <= 0) {
            if (bytesRead == 0) {
                // 客户端正常关闭连接
                // 如果已经收到了部分请求，返回这部分请求
                return request;
            } else {
                // 发生错误
                #ifdef _WIN32
                int error = WSAGetLastError();
                if (error == WSAEWOULDBLOCK) {
                    // 非阻塞模式下暂时没有数据，如果还没有找到头部，继续等待
                    if (!headerFound) {
                        continue;
                    }
                    // 如果已经找到头部，检查请求体是否完整
                    if (contentLength > 0) {
                        size_t bodyStart = request.find("\r\n\r\n") + 4;
                        if (request.length() - bodyStart < contentLength) {
                            // 请求体还没接收完，继续等待
                            continue;
                        }
                    }
                    // 请求体已接收完整，跳出循环
                    break;
                }
                #else
                int error = errno;
                if (error == EAGAIN || error == EWOULDBLOCK) {
                    // 非阻塞模式下暂时没有数据，如果还没有找到头部，继续等待
                    if (!headerFound) {
                        continue;
                    }
                    // 如果已经找到头部，检查请求体是否完整
                    if (contentLength > 0) {
                        size_t bodyStart = request.find("\r\n\r\n") + 4;
                        if (request.length() - bodyStart < contentLength) {
                            // 请求体还没接收完，继续等待
                            continue;
                        }
                    }
                    // 请求体已接收完整，跳出循环
                    break;
                }
                #endif
                // 其他错误，返回空字符串
                return "";
            }
        }
        
        request.append(buffer, bytesRead);
        
        if (!headerFound) {
            // 检查是否收到完整的 HTTP 请求头（寻找 \r\n\r\n）
            headerEnd = request.find("\r\n\r\n");
            if (headerEnd != std::string::npos) {
                headerFound = true;
                
                // 检查是否有 Content-Length 头部
                size_t contentLengthPos = request.find("Content-Length:");
                if (contentLengthPos != std::string::npos && contentLengthPos < headerEnd) {
                    // 提取 Content-Length 值
                    size_t valueStart = request.find(" ", contentLengthPos) + 1;
                    size_t valueEnd = request.find("\r\n", valueStart);
                    std::string contentLengthStr = request.substr(valueStart, valueEnd - valueStart);
                    contentLength = std::stoi(contentLengthStr);
                }
            }
        }
        
        // 如果头部已找到
        if (headerFound) {
            // 如果有 Content-Length，检查请求体是否完整
            if (contentLength > 0) {
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
    return request;
}

/**
 * 辅助函数：发送HTTP响应
 * 功能：向客户端发送HTTP响应
 * 参数：clientSocket - 客户端socket
 *       response - HTTP响应字符串
 * 返回值：成功返回true，失败返回false
 */
static bool sendHttpResponse(SOCKET clientSocket, const std::string& response) {
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
            return false;
        }
        totalSent += bytesSent;
    }
    return true;
}

/**
 * 辅助函数：解析HTTP请求
 * 功能：从HTTP请求中解析方法、路径和请求体
 * 参数：request - HTTP请求字符串
 *       method - 输出参数，存储HTTP方法
 *       path - 输出参数，存储请求路径
 *       body - 输出参数，存储请求体
 */
static void parseHttpRequest(const std::string& request, std::string& method, std::string& path, std::string& body) {
    // 解析HTTP请求的方法和路径
    size_t methodEnd = request.find(" ");
    size_t pathEnd = request.find(" ", methodEnd + 1);
    if (methodEnd != std::string::npos && pathEnd != std::string::npos) {
        method = request.substr(0, methodEnd);
        path = request.substr(methodEnd + 1, pathEnd - methodEnd - 1);
    }
    
    // 提取请求体
    size_t headerEndPos = request.find("\r\n\r\n");
    if (headerEndPos != std::string::npos) {
        body = request.substr(headerEndPos + 4);
    }
}

/**
 * 初始化网络
 * 功能：初始化网络库，为Socket操作做准备
 */
void initializeNetwork() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup initialization failed" << std::endl;
        exit(1);
    }
#endif
}

/**
 * 创建服务器Socket
 * 功能：创建并配置服务器Socket
 * 参数：port - 服务器端口
 * 返回值：创建的服务器Socket
 */
SOCKET createServerSocket(int port) {
    // 创建Socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
#ifdef _WIN32
        WSACleanup();
#endif
        exit(1);
    }

    // 设置服务器地址
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    // 绑定端口
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Port binding failed" << std::endl;
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        exit(1);
    }

    // 开始监听
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        std::cerr << "Listen failed" << std::endl;
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        exit(1);
    }

    // 设置服务器套接字到NetworkManager
    NetworkManager::getInstance().setServerSocket(serverSocket);
    
    // 输出服务器信息
    std::cout << "Server started, port: " << port << std::endl;
    // 替换 inet_ntoa 为 inet_ntop
    char ipBuffer[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &serverAddr.sin_addr, ipBuffer, sizeof(ipBuffer));
    std::cout << "Local IP address: " << ipBuffer << std::endl;
    std::cout << "Enter 'exit', 'quit' or 'stop' to stop the server" << std::endl;

    return serverSocket;
}

/**
 * 处理客户端连接
 * 功能：处理客户端的HTTP请求
 * 参数：clientSocket - 客户端Socket
 */
void handleClientConnection(SOCKET clientSocket) {
    // 接收完整的HTTP请求
    std::string request = receiveHttpRequest(clientSocket);
    if (request.empty()) {
        // 请求接收失败，清理连接
        cleanupClientSocket(clientSocket);
        return;
    }
    
    // 解析HTTP请求的方法、路径和请求体
    std::string method, path, body;
    parseHttpRequest(request, method, path, body);
    
    // 处理HTTP请求
    std::string response = handleHttpRequest(method, path, body);

    // 发送HTTP响应
    sendHttpResponse(clientSocket, response);
    
    // 清理连接
    cleanupClientSocket(clientSocket);
}