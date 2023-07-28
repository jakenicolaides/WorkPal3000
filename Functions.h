#pragma once
#include <Windows.h>
#include <iostream>
#include <typeinfo>
#include <vector>
#include <glm.hpp>

namespace Functions {

     extern std::string root;

     std::string EscapeBackslashes(const std::string& path);
     LPCWSTR ConvertToLPCWSTR(const char* narrowString);
     LPWSTR ConvertToLPWSTR(const std::string & str);
     std::string getFilePathExtension(std::string imagePath);
     bool isSubstring(const std::string& mainstring, const std::string& substring, bool caseSensitive = false);

     std::string insertNewlines(const std::string& str, int line_length);

     int countNewlines(std::string str);

     std::string getFileType(std::string filePath);

     std::string getFileNameFromPath(const std::string& path);

     std::string convertBytes(long long bytes, char choice);

     std::vector<std::string> findFilesWithExtensions(std::vector<std::string>& extensions);
     std::string removeSubstring(std::string originalString, std::string substringToRemove);
     glm::vec3 getWorldPosition(const glm::mat4& matrix);
     std::string vectorToString(glm::vec3 vector);
         
}