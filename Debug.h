#pragma once

#include <vector>
#include <string>
#include <chrono>
#include <glm.hpp>

enum LogSource { Vulkan, Engine, Game };
enum LogType { Message, Warning, Error };

struct LogEntry {
    std::string message;
    std::string logSource;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
    LogType logType;
};

struct Debug {
    void log(std::string message, LogSource source = Engine, LogType type = Message);
    void log(int integer, LogSource source = Engine, LogType type = Message);
    void log(glm::vec3 vector, LogSource source = Engine, LogType type = Message);
    void log(glm::mat4 matrix, LogSource source = Engine, LogType type = Message);
    void clearLog();
    const std::vector<LogEntry>& getLogEntries();
    std::vector<LogEntry> logEntries;
    int logCount = 0;
};

std::string enumToString(LogSource source);
std::string mat4ToString(const glm::mat4& mat);

extern Debug debug;