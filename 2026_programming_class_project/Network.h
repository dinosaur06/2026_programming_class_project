// 네트워크 인터페이스 (윈도우와 리눅스 공통으로 호출할 함수 선언)
#pragma once
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h> // 리눅스 전용 epoll 헤더
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

// 공통 함수 선언
bool InitializeNetwork(); // 네트워크 라이브러리 초기화 (윈도우 전용 로직 포함)
void ShutdownNetwork();   // 네트워크 라이브러리 해제
SOCKET CreateUDPSocket(); // UDP 소켓 생성
