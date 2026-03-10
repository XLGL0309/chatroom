#ifndef NETWORK_H
#define NETWORK_H

#include <string>

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
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#define WSADATA int
#define WSAStartup(a, b) 0
#define WSACleanup() 0
#endif

void initializeNetwork();
SOCKET createServerSocket(int port);
void handleClientConnection(SOCKET clientSocket, const std::string& clientIP);

#endif // NETWORK_H