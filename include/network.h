#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <atomic>    // 包含atomic
#include <unordered_map> // 包含unordered_map
#include <mutex>     // 包含mutex

// 跨平台Socket支持
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#define WSADATA int
#define WSAStartup(a, b) 0
#define WSACleanup() 0
// 全局epoll实例，仅Linux分支
extern int g_epoll_fd;
#endif

// 客户端IP映射，跨平台
extern std::unordered_map<SOCKET, std::string> g_clientIPMap;
extern std::mutex g_clientIPMapMutex;

extern std::atomic<bool> g_running;
extern SOCKET g_serverSocket;

void initializeNetwork();
SOCKET createServerSocket(int port);
void handleClientConnection(SOCKET clientSocket, const std::string& clientIP);

#endif // NETWORK_H