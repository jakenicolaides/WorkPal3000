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
#include "SFML/Audio.hpp"

namespace WorkPal3000 {

    extern bool isIdling;
    extern std::vector<std::string> blocklist;
    extern bool keepRunning;
    extern std::string version;
    extern bool needsOneTimeSetup;
    extern bool invalidLogin;
    extern bool subscriptionActive;
    extern std::string ambientSoundFile;
    extern std::mutex ambientSoundMutex;
    extern int idleDuration;
    extern int intervalDuration;
    extern bool playIntervalSounds;

    void timerThread(const std::chrono::high_resolution_clock::time_point& startTime);
    double getSecondsOnDate(std::string date);
    std::thread startTimer();
    void printElapsedTime();
    std::string getElapsedTime();
    void playAmbientSound();
    void beginThreads();
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
    void validateSubscription(const char* applicationKey);
    void oneTimeSignIn(const char* email, const char* password);
    size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
    void saveTime();
    std::vector<std::string> getDatesInWeek(int weekNumber);
    int currentWeekOfYear();
    void clearHostsFile();
    void getHoursInYear(double yearData[12]);
    void flashWindow();


}