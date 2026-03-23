#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <atomic>    // 包含atomic
#include <unordered_set> // 包含unordered_set
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

// 客户端Socket集合，跨平台
extern std::unordered_set<SOCKET> g_clientSocketSet;
extern std::mutex g_clientSocketSetMutex;

extern std::atomic<bool> g_running;
extern SOCKET g_serverSocket;

void initializeNetwork();
SOCKET createServerSocket(int port);
void handleClientConnection(SOCKET clientSocket);

#endif // NETWORK_H