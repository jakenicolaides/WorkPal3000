#define NOMINMAX
#include <limits>
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <Windows.h>
#include <shellapi.h>
#define WM_APP_QUIT (WM_APP + 1)
#include "Rendering.h"

namespace WorkPal3000 {

    extern bool isIdling;

    void timerThread(const std::chrono::high_resolution_clock::time_point& startTime);
    std::thread startTimer();
    void printElapsedTime();
    std::string getElapsedTime();
    void start();
    void submitIdleResult(bool wasWorking);
    std::string getIdleTime();

}