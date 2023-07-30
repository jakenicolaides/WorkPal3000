#include "WorkPal3000.h"

namespace WorkPal3000 {

    //Global variables
    bool isIdling = false;


    //Private variables
    std::chrono::duration<double> elapsedTime;
    std::chrono::duration<double> totalElapsedTime(0);
    bool keepRunning = true;
    std::mutex cv_m;
    std::condition_variable cv;
    DWORD idleTime;
    int idleDuration = 1;
    
   
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

    DWORD GetIdleTime() {
        LASTINPUTINFO lii;
        lii.cbSize = sizeof(LASTINPUTINFO);
        GetLastInputInfo(&lii);
        return GetTickCount() - lii.dwTime;
    }

    void checkInactivityThread() {
        std::unique_lock<std::mutex> lock(cv_m);
        while (keepRunning) {
            // This will wait for 1 second or until notified to wake up
            if (cv.wait_for(lock, std::chrono::seconds(1), [] { return !keepRunning; })) {
                break;
            }

            // Synchronize access to idleTime
            
            if (isIdling) {
                idleTime += 1000;
            }
            else {
                idleTime = GetIdleTime();
                std::cout << idleTime;
                std::cout << "\n";
            }

            

            if (idleTime >= (idleDuration * 60 * 1000) && !isIdling) {
                // std::lock_guard<std::mutex> lock(mtx); // Lock here too, in case isIdling is shared
                isIdling = true;
                // flashConsoleWindow();
            }
        }
    }

    void submitIdleResult(bool wasWorking) {

        if (wasWorking) {
            isIdling = false;
            idleTime = 0;
        }
        else {
            int totalIdleSeconds;
            {
                totalIdleSeconds = idleTime / 1000; // Convert milliseconds to seconds
                idleTime = 0;
                isIdling = false;
            }
            {
                std::chrono::seconds idleDuration(totalIdleSeconds);
                totalElapsedTime -= idleDuration;
            }
        }
    }

    std::string getElapsedTime() {
        int totalSeconds = static_cast<int>(totalElapsedTime.count());
        int hours = totalSeconds / 3600;
        int minutes = (totalSeconds % 3600) / 60;
        int seconds = totalSeconds % 60;
        std::string time;

        if (hours == 1) {
            time += "1 Hour ";
        }
        else if (hours > 1) {
            time += std::to_string(hours) + " Hours ";
        }

        if (minutes == 1) {
            time += "1 Minute";
        }
        else if (minutes > 1) {
            time += std::to_string(minutes) + " Minutes";
        }

        if (hours == 0 && minutes == 0 && seconds == 1) {
            time += "1 Second";
        }
        else if (hours == 0 && minutes == 0) {
            time += std::to_string(seconds) + " Seconds";
        }

        return time;
    }

    std::string getIdleTime() {

        int totalIdleSeconds;
        totalIdleSeconds = idleTime / 1000; // Convert milliseconds to seconds
        int hours = totalIdleSeconds / 3600;
        int minutes = (totalIdleSeconds % 3600) / 60;
        int seconds = totalIdleSeconds % 60;
        std::string time;

        if (hours == 1) {
            time += "1 Hour ";
        }
        else if (hours > 1) {
            time += std::to_string(hours) + " Hours ";
        }

        if (minutes == 1) {
            time += "1 Minute";
        }
        else if (minutes > 1) {
            time += std::to_string(minutes) + " Minutes";
        }

        if (hours == 0 && minutes == 0 && seconds == 1) {
            time += "1 Second";
        }
        else if (hours == 0 && minutes == 0) {
            time += std::to_string(seconds) + " Seconds";
        }

        return time;
    }

    void start() {

        std::thread timer = startTimer();
        std::thread inactivityChecker(checkInactivityThread);

        inactivityChecker.detach();
        timer.detach();  
    }
}

int main()
{
    WorkPal3000::start();
    Rendering::start();
    return 0;
}