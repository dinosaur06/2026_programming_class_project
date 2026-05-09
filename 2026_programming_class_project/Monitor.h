#pragma once
#include "Monitor.h"
#include <iostream>
#include <thread>
#include <chrono>
using namespace std;

atomic<int> g_packetCount(0);
atomic<long long> g_totalBytes(0);
static bool g_keepRunning = true;

void MonitorLoop() {
    auto lastTime = chrono::steady_clock::now();
    while (g_keepRunning) {
        this_thread::sleep_for(chrono::seconds(1));

        int currentCount = g_packetCount.exchange(0);
        long long currentBytes = g_totalBytes.exchange(0);

        cout << "\n[서버 부하 리포트]" << endl;
        cout << " - PPS: " << currentCount << " pkts/s" << endl;
        cout << " - Traffic: " << (currentBytes / 1024.0) << " KB/s" << endl;
    }
}

void StartMonitoring() {
    thread t(MonitorLoop);
    t.detach(); // 독립적으로 실행
}

void StopMonitoring() {
    g_keepRunning = false;
}