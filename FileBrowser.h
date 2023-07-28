#pragma once

#include <vector>
#include <fstream>
#include <filesystem>

namespace FileBrowser {

    struct DirectoryItem {
        std::string id;
        std::string name;
        std::string path;
        std::string parentPath;
        bool isFolder;
    };

    struct FolderInfo {
        std::string name;
        int numFolders;
        int numFiles;
        std::string dateModified;
        std::string dateCreated;
    };

    struct ModelInfo {
        std::string name;
        std::string type;
        int numPolygons;
        int numVerts;
        int numMaterials;
        long long fileSize;
    };


    
    std::vector<DirectoryItem> getGameFiles(const std::string& pathToGameFiles);
    void printGameFiles(std::vector<DirectoryItem> gameFiles);
    int LaunchBlenderWithModel(std::string fbxPath);

    bool renameFolder(std::string& path, const std::string& newName);

    std::string getParentDirectory(std::string path);

    bool deleteFileOrFolder(std::wstring path);

    bool createDirectory(const std::string& path);

    FolderInfo getFolderInfo(const std::string& path);

    ModelInfo getModelInfo(const std::string& path);
   

}