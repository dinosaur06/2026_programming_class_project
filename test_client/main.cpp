#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Protocol.h" // 서버와 약속한 패킷 규격
using namespace std;

// 윈도우 전용 네트워크 라이브러리 연결
#pragma comment(lib, "ws2_32.lib")

int main() {
    // 1. 윈도우 소켓 초기화
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "WSAStartup 실패" << endl;
        return -1;
    }

    // 2. UDP 소켓 생성
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        cout << "소켓 생성 실패" << endl;
        WSACleanup();
        return -1;
    }

    // 3. 서버(우분투 가상머신) 주소 설정
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9000); // 서버 포트와 동일하게 설정

    // ★ 여기에 우분투 터미널에서 hostname -I로 확인한 IP를 넣으세요! ★
    // 예: "192.168.0.15"
    inet_pton(AF_INET, "192.168.45.114", &serverAddr.sin_addr);

    // 4. 전송할 패킷 구성 (Protocol.h 활용)
    char buffer[sizeof(PacketHeader) + sizeof(EchoPayload)] = { 0 };
    PacketHeader* header = (PacketHeader*)buffer;

    // Fortress 프로젝트의 핵심: 매직 넘버 검증
    header->magic = 0x46545253; // 'FTRS'
    header->type = 1;           // Echo Request 타입
    header->length = sizeof(EchoPayload);

    EchoPayload* payload = (EchoPayload*)(buffer + sizeof(PacketHeader));
    strcpy_s(payload->message, "Hello from Windows!");

    // 5. 서버로 패킷 전송
    sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    cout << "서버로 패킷을 보냈습니다. IP: " << "192.168.45.114" << endl;

    // 6. 서버로부터 에코(응답) 받기 대기
    char recvBuf[sizeof(buffer)];
    int addrLen = sizeof(serverAddr);
    int recvLen = recvfrom(sock, recvBuf, sizeof(recvBuf), 0, (struct sockaddr*)&serverAddr, &addrLen);

    if (recvLen > 0) {
        EchoPayload* resPayload = (EchoPayload*)(recvBuf + sizeof(PacketHeader));
        cout << "서버로부터 응답 도착: " << resPayload->message << endl;
    }
    else {
        cout << "응답을 받지 못했습니다. 네트워크 설정을 확인하세요." << endl;
    }

    // 7. 종료
    closesocket(sock);
    WSACleanup();

    system("pause"); // 결과창이 바로 닫히지 않게 함
    return 0;
}