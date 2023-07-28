#pragma once

#ifndef HEADER_FILE_H
#define HEADER_FILE_H

#include <map>
#include <fstream>
#include "FileBrowser.h"
#include "Rendering.h"

#include <imgui.h>

namespace EngineUI {

	extern std::string currentSelectedItem;

	enum BrowserFocus {
		FileBrowser,
		FolderBrowser
	};
	
	void updateModelTransform(glm::mat4& transformMatrix, int entityIndex);
	void renderFolderTree(std::string parentPath, bool clickCaptured);
	std::map<std::string, bool> loopUI(bool& applicationOpen, float &cameraSpeed, float &fov, bool& firstMouse);
	void initUI();
	bool getIsNavigatingViewport();
	void openDirectory(std::string path, BrowserFocus focus);
	void prepareNewInspectorImage(std::string imagePath);
	void switchNewInsectorImage();
	void openPathInFileExplorer(std::string path);
	static int fileSearchCallback(ImGuiInputTextCallbackData* data);
	static int newTypeCallback(ImGuiInputTextCallbackData* data);
	static int typeModelSearchCallback(ImGuiInputTextCallbackData* data);
	void renderLog();
	void showRightClickDirectoryOptions(FileBrowser::DirectoryItem item);
	float estimateTextWidth(const char* text);
      
}

#endif