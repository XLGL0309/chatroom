#include "../include/network.h"
#include "../include/web.h"
#include <iostream>
#include <thread>
#ifdef __linux__
#include <fcntl.h>
#else
#include <ws2tcpip.h> // 为Windows提供inet_ntop
#endif

// NetworkManager实现
NetworkManager& NetworkManager::getInstance() {
    static NetworkManager instance;
    return instance;
}

std::unordered_set<SOCKET>& NetworkManager::getClientSocketSet() {
    return clientSocketSet;
}

std::mutex& NetworkManager::getClientSocketSetMutex() {
    return clientSocketSetMutex;
}

std::atomic<bool>& NetworkManager::getRunning() {
    return running;
}

void NetworkManager::setRunning(bool value) {
    running = value;
}

SOCKET& NetworkManager::getServerSocket() {
    return serverSocket;
}

void NetworkManager::setServerSocket(SOCKET socket) {
    serverSocket = socket;
}

#ifdef __linux__
int NetworkManager::getEpollFd() {
    return epoll_fd;
}

void NetworkManager::setEpollFd(int fd) {
    epoll_fd = fd;
}
#endif

// 辅助函数：清理客户端socket（从epoll移除并关闭）
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

// 辅助函数：接收完整的HTTP请求
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

// 辅助函数：发送HTTP响应
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

// 辅助函数：解析HTTP请求
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

    NetworkManager::getInstance().setServerSocket(serverSocket);
    std::cout << "Server started, port: " << port << std::endl;
    // 替换 inet_ntoa 为 inet_ntop
    char ipBuffer[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &serverAddr.sin_addr, ipBuffer, sizeof(ipBuffer));
    std::cout << "Local IP address: " << ipBuffer << std::endl;
    std::cout << "Enter 'exit', 'quit' or 'stop' to stop the server" << std::endl;

    return serverSocket;
}

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