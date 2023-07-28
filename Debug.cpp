#include "Debug.h"
#include <vector>

#include <string>
#include <chrono>



void Debug::log(std::string message, LogSource source, LogType type)
{
    LogEntry newLogEntry;
    newLogEntry.message = message;
    newLogEntry.logSource = enumToString(source),
    newLogEntry.timestamp = std::chrono::system_clock::now();
    newLogEntry.logType = type;
    logEntries.push_back(newLogEntry);
    return;
}

void Debug::log(int integer, LogSource source, LogType type)
{
    LogEntry newLogEntry;
    std::string message = std::to_string(integer);
    newLogEntry.message = message;
    newLogEntry.logSource = enumToString(source),
        newLogEntry.timestamp = std::chrono::system_clock::now();
    newLogEntry.logType = type;
    logEntries.push_back(newLogEntry);
    return;
}

void Debug::log(glm::vec3 vector, LogSource source, LogType type)
{
    LogEntry newLogEntry;
    std::string message = "X: " + std::to_string(vector.x) + " Y: " + std::to_string(vector.y) + " Z: " + std::to_string(vector.z);
    newLogEntry.message = message;
    newLogEntry.logSource = enumToString(source),
        newLogEntry.timestamp = std::chrono::system_clock::now();
    newLogEntry.logType = type;
    logEntries.push_back(newLogEntry);
    return;
}

void Debug::log(glm::mat4 matrix, LogSource source, LogType type)
{
    LogEntry newLogEntry;
    std::string message= mat4ToString(matrix);
    newLogEntry.message = message;
    newLogEntry.logSource = enumToString(source),
    newLogEntry.timestamp = std::chrono::system_clock::now();
    newLogEntry.logType = type;
    logEntries.push_back(newLogEntry);
    return;
}

const std::vector<LogEntry>& Debug::getLogEntries()
{
    return logEntries;
}

std::string enumToString(LogSource source)
{
    switch (source) {
    case Vulkan:
        return "Vulkan";
    case Engine:
        return "Engine";
    case Game:
        return "Game";
    }
}

void Debug::clearLog() {

    logEntries.clear();
    return;

}


std::string mat4ToString(const glm::mat4& mat) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "[" << mat[0][0] << ", " << mat[0][1] << ", " << mat[0][2] << ", " << mat[0][3] << "]" << std::endl;
    oss << "[" << mat[1][0] << ", " << mat[1][1] << ", " << mat[1][2] << ", " << mat[1][3] << "]" << std::endl;
    oss << "[" << mat[2][0] << ", " << mat[2][1] << ", " << mat[2][2] << ", " << mat[2][3] << "]" << std::endl;
    oss << "[" << mat[3][0] << ", " << mat[3][1] << ", " << mat[3][2] << ", " << mat[3][3] << "]";
    return oss.str();
}

Debug debug;


