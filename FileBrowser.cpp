#include "FileBrowser.h"
#include "Functions.h"
#include "nlohmann/json.hpp"
#include "Debug.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <regex>


namespace FileBrowser {


    std::vector<DirectoryItem> getGameFiles(const std::string& pathToGameFiles) {
        std::vector<DirectoryItem> gameFiles;

        //Add the root folder
        DirectoryItem root = {};
        root.id = "root"; // arbitrary ID for the root folder
        root.name = "Content";
        root.path = std::filesystem::current_path().string() + "\\content";
        root.parentPath = "";
        root.isFolder = true;

        gameFiles.push_back(root);

        // Create a new instance of recursive_directory_iterator
        for (const auto& entry : std::filesystem::recursive_directory_iterator(pathToGameFiles)) {
            DirectoryItem item = {};
            item.name = entry.path().filename().string();
            item.path = entry.path().string();
            item.parentPath = entry.path().parent_path().string();
            item.isFolder = entry.is_directory();

            // Get a unique ID for the file/folder using Windows API function GetFileInformationByHandle
            if (!entry.is_directory()) {
                HANDLE fileHandle = CreateFile(
                    entry.path().c_str(),
                    GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                );
                if (fileHandle != INVALID_HANDLE_VALUE) {
                    BY_HANDLE_FILE_INFORMATION fileInfo = {};
                    if (GetFileInformationByHandle(fileHandle, &fileInfo)) {
                        item.id = std::to_string(fileInfo.nFileIndexLow) + "_" + std::to_string(fileInfo.nFileIndexHigh);
                    }
                    CloseHandle(fileHandle);
                }
            }
            else {
                item.id = std::to_string(std::hash<std::string>{}(item.path));
            }

            gameFiles.push_back(item);
        }

        return gameFiles;
    }

    

    void printGameFiles(std::vector<DirectoryItem> gameFiles) {
        
        for (DirectoryItem& item : gameFiles) {
            std::cout << "Id: " << item.id << std::endl;
            std::cout << "Name: " << item.name << std::endl;
            std::cout << "Path: " << item.path << std::endl;
            std::cout << "Parent Path: " << item.parentPath<< std::endl;
            std::cout << "Is Folder: " << item.isFolder << std::endl;
            std::cout << " " << std::endl;
        }
    }

    int LaunchBlenderWithModel(std::string modelPath)
    {
        // Add the path to the Blender executable to the PATH environment variable
        std::string blenderPath = "C:\\Program Files\\Blender Foundation\\Blender 3.4";
        char* oldValue;
        size_t len;
        _dupenv_s(&oldValue, &len, "PATH");
        std::string pathEnvVar = blenderPath + ";" + oldValue;
        LPCWSTR widePath = Functions::ConvertToLPCWSTR(pathEnvVar.c_str());
        SetEnvironmentVariable(L"PATH", widePath);
        free(oldValue);

        // Construct the command string with the updated FBX path
        std::string command = "blender --python \""+Functions::root+"\\auto_open.py\" -- \"" + modelPath + "\"";
        LPWSTR const n = Functions::ConvertToLPWSTR(command.c_str());
        // Launch Blender in a new process
        STARTUPINFO si = { sizeof(si) };
        PROCESS_INFORMATION pi;
       
        if (!CreateProcess(NULL,n, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            // Failed to create process
            return -1;
        }
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);

        return 0;
    }

    bool renameFolder(std::string& path, const std::string& newName) {


        // Construct the old and new folder paths using the folder ID and new name
        std::wstring oldFolderPath = Functions::ConvertToLPWSTR(path);
        std::string newPathString = getParentDirectory(path) + "\\" + newName;
        std::wstring newFolderPath = Functions::ConvertToLPWSTR(newPathString);

        // Rename the folder using the Windows API
        if (MoveFileEx(oldFolderPath.c_str(), newFolderPath.c_str(), MOVEFILE_REPLACE_EXISTING)) {
            return true;
        }

        // Handle errors
        DWORD error = GetLastError();
        LPWSTR buffer = nullptr;
        size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&buffer, 0, nullptr);
        if (size > 0) {
            std::wcerr << L"Error " << error << L": " << buffer << std::endl;
            LocalFree(buffer);
        }
        else {
            std::wcerr << L"Unknown error occurred." << std::endl;
        }

        return false;


    }

    std::string getParentDirectory(std::string path) {
        size_t pos = path.find_last_of("\\/");
        if (pos != std::string::npos) {
            return path.substr(0, pos);
        }
        return "";
    }

    bool deleteFileOrFolder(std::wstring path)
    {
        DWORD attributes = GetFileAttributesW(path.c_str());

        if (attributes == INVALID_FILE_ATTRIBUTES)
        {
            std::cerr << "Error: could not get file attributes for path " << std::string(path.begin(), path.end()) << std::endl;
            return false;
        }

        if (attributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            // Delete all files and subfolders within the folder
            std::wstring searchPath = path + L"\\*";
            WIN32_FIND_DATAW findData;
            HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
            if (hFind != INVALID_HANDLE_VALUE)
            {
                do
                {
                    if (wcscmp(findData.cFileName, L".") != 0 && wcscmp(findData.cFileName, L"..") != 0)
                    {
                        std::wstring filePath = path + L"\\" + findData.cFileName;
                        if (!deleteFileOrFolder(filePath))
                        {
                            FindClose(hFind);
                            return false;
                        }
                    }
                } while (FindNextFileW(hFind, &findData));

                FindClose(hFind);
            }

            // Delete the empty folder
            if (RemoveDirectoryW(path.c_str()))
            {
                return true;
            }
            else
            {
                std::cerr << "Error: could not remove directory at path " << std::string(path.begin(), path.end()) << std::endl;
                return false;
            }
        }
        else
        {
            if (DeleteFileW(path.c_str()))
            {
                return true;
            }
            else
            {
                std::cerr << "Error: could not delete file at path " << std::string(path.begin(), path.end()) << std::endl;
                return false;
            }
        }
    }


    bool createDirectory(const std::string& path)
    {
        if (CreateDirectory(Functions::ConvertToLPCWSTR(path.c_str()), NULL) || ERROR_ALREADY_EXISTS == GetLastError())
        {
            return true;
        }
        else
        {
            DWORD errorCode = GetLastError();
            std::cout << "Error creating directory: " << path << std::endl;
            LPSTR messageBuffer = nullptr;
            size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
            std::string message(messageBuffer, size);
            std::cout << "Error code: " << errorCode << " - " << message << std::endl;
            LocalFree(messageBuffer);
            return false;
        }
    }

    FolderInfo getFolderInfo(const std::string& path) {
        FolderInfo info;
        
        
        WIN32_FIND_DATAA data;
        HANDLE hFind = FindFirstFileA((path + "/*").c_str(), &data);
        if (hFind == INVALID_HANDLE_VALUE) {
            std::cerr << "Error: Invalid path\n";
            return FolderInfo();
        }

        info.name = path.substr(path.find_last_of('\\') + 1);
        info.numFolders = 0;
        info.numFiles = 0;

        SYSTEMTIME modTime, createTime;
        FileTimeToSystemTime(&data.ftLastWriteTime, &modTime);
        FileTimeToSystemTime(&data.ftCreationTime, &createTime);
        std::stringstream modTimeSS, createTimeSS;
        modTimeSS << modTime.wYear << "-" << modTime.wMonth << "-" << modTime.wDay << " " << modTime.wHour << ":" << modTime.wMinute << ":" << modTime.wSecond;
        createTimeSS << createTime.wYear << "-" << createTime.wMonth << "-" << createTime.wDay << " " << createTime.wHour << ":" << createTime.wMinute << ":" << createTime.wSecond;
        info.dateModified = modTimeSS.str();
        info.dateCreated = createTimeSS.str();

        do {
            if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (std::strcmp(data.cFileName, ".") != 0 && std::strcmp(data.cFileName, "..") != 0) {
                    info.numFolders++;
                }
            }
            else {
                info.numFiles++;
            }
        } while (FindNextFileA(hFind, &data) != 0);

        FindClose(hFind);
        return info;
        
    }


    ModelInfo getModelInfo(const std::string& path) {

        ModelInfo info;

        std::ifstream file(path, std::ios::binary);
        if (!file) {
            debug.log("Error: could not open file " + path);

        }

        // Get file size in bytes
        long long fileSize = std::filesystem::file_size(path);
        info.name = path.substr(path.find_last_of("\\/") + 1);;
        info.type = path.substr(path.find_last_of('.') + 1);
        info.fileSize = fileSize;
        info.numMaterials = 0;
        info.numPolygons = 0;
        info.numVerts = 0;

        if (info.type == "glb") {

            // Read the GLB header to get the total length of the file
            std::vector<char> header(12);
            file.read(header.data(), 12);
            uint32_t totalLength = *reinterpret_cast<uint32_t*>(&header[8]);

            // Find the JSON chunk and parse it to get the number of vertices, polygons, and material slots
            char chunkType[5];
            uint32_t chunkLength;
            while (file.tellg() < totalLength) {
                file.read(chunkType, 4);
                chunkType[4] = '\0';
                file.read(reinterpret_cast<char*>(&chunkLength), 4);

                if (std::string(chunkType) == "JSON") {
                    std::vector<char> jsonChunk(chunkLength);
                    file.read(jsonChunk.data(), chunkLength);
                    nlohmann::json j = nlohmann::json::parse(jsonChunk);

                    if (j.contains("meshes")) {
                        for (auto& mesh : j["meshes"]) {
                            if (mesh.contains("primitives")) {
                                for (auto& primitive : mesh["primitives"]) {
                                    if (primitive.contains("attributes")) {
                                        int numVerts = 0;
                                        if (primitive["attributes"].contains("POSITION")) {
                                            int accessorId = primitive["attributes"]["POSITION"];
                                            numVerts = j["accessors"][accessorId]["count"];
                                        }
                                        info.numVerts += numVerts;

                                        int numIndices = 0;
                                        if (primitive.contains("indices")) {
                                            int accessorId = primitive["indices"];
                                            numIndices = j["accessors"][accessorId]["count"];
                                        }
                                        int numPolys = numIndices / 3;
                                        info.numPolygons += numPolys;

                                        int numMaterials = 0;
                                        if (primitive.contains("material")) {
                                            numMaterials = 1;
                                        }
                                        info.numMaterials += numMaterials;
                                    }
                                }
                            }
                        }
                    }
                    break;
                }

                // Skip the chunk data
                file.seekg(chunkLength, std::ios::cur);
            }

        }
        else if (info.type == "gltf") {
            std::stringstream ss;
            ss << file.rdbuf();
            nlohmann::json j = nlohmann::json::parse(ss.str());

            if (j.contains("meshes")) {
                for (auto& mesh : j["meshes"]) {
                    if (mesh.contains("primitives")) {
                        for (auto& primitive : mesh["primitives"]) {
                            if (primitive.contains("attributes")) {
                                int numVerts = 0;
                                if (primitive["attributes"].contains("POSITION")) {
                                   
                                    int accessorId = primitive["attributes"]["POSITION"];
                                    numVerts = j["accessors"][accessorId]["count"];
                                }
                                info.numVerts += numVerts;
                                
                                int numIndices = 0;
                                if (primitive.contains("indices")) {
                                    int accessorId = primitive["indices"];
                                    numIndices = j["accessors"][accessorId]["count"];;
                                }
                                int numPolys = numIndices / 3;
                                info.numPolygons += numPolys;
                                

                                int numMaterials = 0;
                                if (primitive.contains("material")) {
                                    numMaterials = 1;
                                }
                                info.numMaterials += numMaterials;
                            }
                        }
                    }
                }
            }
        }
        

        return info;
        
    }



}