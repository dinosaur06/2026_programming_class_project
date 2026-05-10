#include "Monitor.h"
#include <iostream>
#include <thread>  // 모니터링 루프를 1초마다 돌리기 위해 필요
#include <chrono>  // 모니터링 루프를 1초마다 돌리기 위해 필요
#include <vector>  // g_history에 필요
#include <fstream> // 파일 입출력을 위해 추가
#include <iomanip> // 'setw'를 사용하기 위해 필요
using namespace std;

atomic<int> g_packetCount(0);
atomic<long long> g_totalBytes(0);
static bool g_keepRunning = true;

// 통계 기록을 위한 벡터
struct Stats {
    int second;      // 서버 가동 후 경과 시간(초)
    int pps;
    double kbps;
};
vector<Stats> g_history;

void MonitorLoop() {
    int elapsedSeconds = 0;
    while (g_keepRunning) {
        // 1초 대기
        this_thread::sleep_for(chrono::seconds(1));

        // 데이터 추출 및 초기화
        int currentCount = g_packetCount.exchange(0);
        long long currentBytes = g_totalBytes.exchange(0);
        double currentKB = currentBytes / 1024.0;

        // 시점과 함께 기록
        g_history.push_back({ elapsedSeconds, currentCount, currentKB });

        // 실시간 알람 (특정 값 이상일 때만 화면에 노출)
        if (currentCount > 1000) {
            cout << "\a[!] 경고: 초당 패킷 급증! [" << elapsedSeconds << "초 지점] PPS: " << currentCount << endl;
        }
    }
}

// 서버 종료 시 호출될 최종 리포트 함수
void PrintFinalReport() {
    long long grandTotalPackets = 0;
    int maxPPS = 0;
    int maxPPSAt = 0;
    int threshold = 500; // 시점을 기록할 특정 임계치
    vector<int> thresholdReachedTimes;

    for (const auto& s : g_history) {
        grandTotalPackets += s.pps;

        // 1. 최대 PPS 시점 찾기
        if (s.pps > maxPPS) {
            maxPPS = s.pps;
            maxPPSAt = s.second;
        }

        // 2. 특정 값(임계치) 이상이었던 모든 시점 기록
        if (s.pps >= threshold) {
            thresholdReachedTimes.push_back(s.second);
        }
    }

    // 콘솔 화면에 즉시 요약 리포트 출력
    cout << "\n" << string(40, '=') << endl;
    cout << "       [ 서버 보안 분석 결과 ]" << endl;
    cout << string(40, '=') << endl;
    cout << " - 서버 가동 시간: " << g_history.size() << "초" << endl;
    cout << " - 누적 수신 패킷: " << grandTotalPackets << " 개" << endl;
    cout << " - 피크 수치: " << maxPPS << " PPS (발생: " << maxPPSAt << "초)" << endl;

    if (!thresholdReachedTimes.empty()) {
        cout << " - 이상 징후: " << thresholdReachedTimes.size() << "회 감지됨" << endl;
    }
    else {
        cout << " - 이상 징후: 없음" << endl;
    }
    cout << string(40, '=') << endl;

    // --- 텍스트 파일 저장 로직 ---
    ofstream outFile("security_report.txt");
    if (outFile.is_open()) {
        outFile << "========================================" << endl;
        outFile << "       서버 보안 정밀 분석 리포트" << endl;
        outFile << "========================================" << endl;
        outFile << "1. 총 수신 패킷: " << grandTotalPackets << " 개" << endl;
        outFile << "2. 최고 피크 PPS: " << maxPPS << " pkts/s (발생 시점: " << maxPPSAt << "초)" << endl;
        outFile << "\n3. 임계치(" << threshold << " PPS) 초과 기록:" << endl;
        if (thresholdReachedTimes.empty()) {
            outFile << " - 감지된 이상 징후 없음" << endl;
        }
        else {
            for (int t : thresholdReachedTimes) {
                // 해당 시점의 상세 pps를 다시 찾아서 출력
                outFile << " - [" << t << "초] 지점에서 임계치 초과 발생" << endl;
            }
        }

        outFile << "\n[전체 타임라인 데이터]" << std::endl;
        outFile << "시간(초) | PPS | 트래픽(KB/s)" << std::endl;
        for (const auto& s : g_history) {
            outFile << setw(6) << s.second << " | " << setw(5) << s.pps << " | " << s.kbps << endl;
        }
        outFile.close();
        cout << "\n[+] 리포트가 security_report.txt 파일이 생성되었습니다." << endl;
    }
    g_history.clear(); // 데이터 초기화
}

void StartMonitoring() {
    g_keepRunning = true;
    thread t(MonitorLoop);
    t.detach();
}

void StopMonitoring() {
    g_keepRunning = false;
    this_thread::sleep_for(chrono::milliseconds(500));
    PrintFinalReport(); // 종료 시 리포트 출력
}