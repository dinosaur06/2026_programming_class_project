#pragma once
#include <atomic>
using namespace std;

// 다른 파일에서 접근 가능하도록 공유 변수 선언
extern atomic<int> g_packetCount;
extern atomic<long long> g_totalBytes;

// 모니터링 스레드 시작 함수
void StartMonitoring();
void StopMonitoring();