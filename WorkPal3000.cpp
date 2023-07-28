#include "WorkPal3000.h"
#include <iostream>
#include <chrono>
#include <thread>

namespace WorkPal3000 {

    std::chrono::duration<double> elapsedTime; 
    std::chrono::duration<double> totalElapsedTime(0);


    void timerThread(const std::chrono::high_resolution_clock::time_point& startTime) {
        std::chrono::high_resolution_clock::time_point lastTime = startTime;

        while (true) {
            // This will wait for 1 second or until notified to wake up
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - lastTime);
            totalElapsedTime += elapsedTime;

            lastTime = currentTime; // Update lastTime for the next iteration
        }
    }

    std::thread startTimer() {
        std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();
        std::thread timer(timerThread, startTime);
        return timer;
    }

    void printElapsedTime() {
        int totalSeconds = static_cast<int>(totalElapsedTime.count());
        int hours = totalSeconds / 3600;
        int minutes = (totalSeconds % 3600) / 60;
        int seconds = totalSeconds % 60;
        std::cout << "You have been working for: " << hours << " hours, " << minutes << " minutes, " << seconds << " seconds\n";
    }

    std::string getElapsedTime() {
        int totalSeconds = static_cast<int>(totalElapsedTime.count());
        int hours = totalSeconds / 3600;
        int minutes = (totalSeconds % 3600) / 60;
        int seconds = totalSeconds % 60;
        std::string time = "  You have been working for\n" + std::to_string(hours) + " hours, " + std::to_string(minutes)+" minutes, "+ std::to_string(seconds)+" seconds\n";
        return time;
    }

    void start() {
        std::thread timer = startTimer();
        timer.detach();  // Detach the thread to let it run independently.
    }
}

int main()
{
    WorkPal3000::start();
    Rendering::start();
    return 0;
}