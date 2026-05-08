#include "Network.h"
#include <iostream>
using namespace std;

bool InitializeNetwork() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed." << endl;
        return false;
    }
#endif
    return true;
}

void ShutdownNetwork() {
#ifdef _WIN32
    WSACleanup();
#endif
}

SOCKET CreateUDPSocket() {
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        cerr << "Failed to create socket." << endl;
    }
    return sock;
}