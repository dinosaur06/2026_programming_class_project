#include <iostream>
#include <vector>
#include <chrono>
#include <csignal> // 신호 처리를 위해 추가
#include "Network.h"
#include "Protocol.h"
#include "Monitor.h" // 모니터링 기능 포함
using namespace std;

// Ctrl + C를 눌렀을 때 실행될 함수
void handle_sigint(int sig) {
    cout << "\n[!] 서버 종료 신호를 받았습니다. 리포트를 생성합니다..." << endl;
    StopMonitoring(); // 여기서 리포트가 출력됩니다!
    exit(sig);
}

int main() {
    // 1. 네트워크 라이브러리 초기화
    if (!InitializeNetwork()) {
        return -1;
    }

    // 2. UDP 소켓 생성
    SOCKET serverSock = CreateUDPSocket();
    if (serverSock == INVALID_SOCKET) {
        ShutdownNetwork();
        return -1;
    }

    // 3. 서버 주소 설정 및 바인딩 (포트 9000번 사용)
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(9000);

    if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Bind failed." << endl;
        ShutdownNetwork();
        return -1;
    }

    cout << "========================================" << endl;
    cout << "   Fortress Game Server Started" << endl;
    cout << "   Port: 9000 | Mode: UDP" << endl;
    cout << "========================================" << endl;

    // 신호 등록: Ctrl + C(SIGINT)가 들어오면 handle_sigint를 실행하라
    signal(SIGINT, handle_sigint);

    // [중요] 4. 모니터링 스레드 시작 (별도 스레드에서 1초마다 출력)
    StartMonitoring();

    // 5. 에코 및 수신 루프
    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);

        // 패킷을 담을 버퍼 준비
        char buffer[sizeof(PacketHeader) + sizeof(EchoPayload)];

        // 데이터 수신 (패킷이 올 때까지 여기서 대기)
        int recvLen = recvfrom(serverSock, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &clientLen);

        if (recvLen > 0) {
            // [통계 업데이트] Monitor.cpp에 선언된 전역 변수 값을 증가시킴
            g_packetCount++;
            g_totalBytes += recvLen;

            // 수신된 데이터를 패킷 구조체로 해석
            PacketHeader* header = (PacketHeader*)buffer;

            // [검증] 매직 넘버 확인
            if (header->magic == 0x46545253) { // 'FTRS'
                // 에코 응답 (필요 시 로그를 주석 처리하면 화면이 더 깔끔합니다)
                // cout << "[LOG] Valid Packet - Type: " << header->type << endl;
                sendto(serverSock, buffer, recvLen, 0, (struct sockaddr*)&clientAddr, clientLen);
            }
            else {
                // 잘못된 패킷은 통계에는 잡히지만 처리는 하지 않음
                // cout << "[WARN] Invalid Magic Number!" << endl;
            }
        }
    }

    // 6. 종료
    StopMonitoring(); // 스레드 종료 신호
    ShutdownNetwork();
    return 0;
}