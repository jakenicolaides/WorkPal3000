#include "Rendering.h"
#include <iostream>
#include <chrono>
#include <thread>

namespace WorkPal3000 {

    void timerThread(const std::chrono::high_resolution_clock::time_point& startTime);
    std::thread startTimer();
    void printElapsedTime();
    std::string getElapsedTime();
    void start();

}