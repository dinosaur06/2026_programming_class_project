// 패킷 설계 
#pragma once
#include <cstdint>

#pragma pack(push, 1) // 데이터 패딩을 없애서 윈도우/리눅스 간 크기를 통일합니다.

// 패킷 헤더
struct PacketHeader {
    uint32_t magic;      // 패킷 검증용 매직 넘버 (예: 0x46545253 - 'FTRS')
    uint16_t type;       // 패킷 타입 (예: 1=에코 요청, 2=에코 응답)
    uint16_t length;     // 페이로드의 길이
};

// 에코 패킷 페이로드
struct EchoPayload {
    char message[256];   // 실제 주고받을 메시지 데이터
};

#pragma pack(pop)