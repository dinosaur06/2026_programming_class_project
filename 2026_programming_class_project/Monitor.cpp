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
            cout << "\a[!] Warning: Packet per second spike at [" << elapsedSeconds << "seconds. PPS:" << currentCount << endl;
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
    cout << "         [ Server Security Analysis Results ]" << endl;
    cout << string(40, '=') << endl;
    cout << " - Server Uptime: " << g_history.size() << "sec" << endl;
    cout << " - Total Packets Received: " << grandTotalPackets << endl;
    cout << " - Peak Value: " << maxPPS << " PPS (At: " << maxPPSAt << "sec)" << endl;

    if (!thresholdReachedTimes.empty()) {
        cout << " - Anomalies Detected: " << thresholdReachedTimes.size() << "회 감지됨" << endl;
    }
    else {
        cout << " - Anomalies Detected: None" << endl;
    }
    cout << string(40, '=') << endl;

    // --- 텍스트 파일 저장 로직 ---
    ofstream outFile("security_report.txt");
    if (outFile.is_open()) {
        outFile << "========================================" << endl;
        outFile << "       Server Security Detailed Report" << endl;
        outFile << "========================================" << endl;
        outFile << "1. Total Packets Received: " << grandTotalPackets << " 개" << endl;
        outFile << "2. Peak PPS Value: " << maxPPS << " pkts/s (At: " << maxPPSAt << "sec)" << endl;
        outFile << "\n3. Threshold(" << threshold << " PPS) Exceedance Records:" << endl;
        if (thresholdReachedTimes.empty()) {
            outFile << " - No anomalies detected" << endl;
        }
        else {
            for (int t : thresholdReachedTimes) {
                // 해당 시점의 상세 pps를 다시 찾아서 출력
                outFile << " - [At " << t << "s] Threshold exceeded" << endl;
            }
        }

        outFile << "\n[Full Timeline Data]" << endl;
        outFile << "Time(s) | PPS | Traffic(KB/s)" << endl;
        for (const auto& s : g_history) {
            outFile << setw(6) << s.second << " | " << setw(5) << s.pps << " | " << s.kbps << endl;
        }
        outFile.close();
        cout << "\n[+] Report successfully saved to 'security_report.txt'." << endl;
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