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
#include "EngineUI.h"

namespace WorkPal3000 {

    extern bool isIdling;
    extern std::vector<std::string> blocklist;
    extern bool keepRunning;
    extern std::string version;

    void timerThread(const std::chrono::high_resolution_clock::time_point& startTime);
    double getSecondsOnDate(std::string date);
    std::thread startTimer();
    void printElapsedTime();
    std::string getElapsedTime();
    void start();
    void submitIdleResult(bool wasWorking);
    std::string getIdleTime();
    void addUrlsToHostsFile(const std::vector<std::string>& urls);
    void getUserData();
    bool isUrlInHostsFile(const std::string& url);
    void removeUrlFromBlockList(std::string url);
    void removeUrlsFromHostsFile(const std::vector<std::string>& urls);
    void addUrlToBlockList(std::string url, int i);
    void submitBlocklist(std::vector<std::string> newBlocklist);
    void setup();
    void saveTime();
    std::vector<std::string> getDatesInWeek(int weekNumber);
    int currentWeekOfYear();
    void clearHostsFile();
    void getHoursInYear(double yearData[12]);
    void flashWindow();


}