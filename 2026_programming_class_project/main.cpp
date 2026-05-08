#include <iostream>
#include <vector>
#include "Network.h"
#include "Protocol.h"
using namespace std;

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
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // 모든 IP로부터 수신
    serverAddr.sin_port = htons(9000);              // 사용할 포트 번호

    if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Bind failed." << endl;
        ShutdownNetwork();
        return -1;
    }

    cout << "Echo Server started on port 9000..." << endl;

    // 4. 에코 루프
    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);

        // 패킷을 담을 버퍼 준비 (헤더 + 페이로드 크기만큼)
        char buffer[sizeof(PacketHeader) + sizeof(EchoPayload)];

        // 데이터 수신
        int recvLen = recvfrom(serverSock, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &clientLen);

        if (recvLen > 0) {
            // 수신된 데이터를 패킷 구조체로 해석
            PacketHeader* header = (PacketHeader*)buffer;

            // [검증] 매직 넘버가 일치하는지 확인 (DDoS 방어의 기초)
            if (header->magic == 0x46545253) { // 'FTRS'
                cout << "Received valid packet. Type: " << header->type << endl;

                // 받은 데이터를 그대로 다시 전송 (에코)
                sendto(serverSock, buffer, recvLen, 0, (struct sockaddr*)&clientAddr, clientLen);
            }
            else {
                cout << "Invalid packet received. Ignoring..." << endl;
            }
        }
    }

    // 5. 종료
    ShutdownNetwork();
    return 0;
}