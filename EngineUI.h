#pragma once

#ifndef HEADER_FILE_H
#define HEADER_FILE_H

#include <map>
#include <fstream>
#include "Rendering.h"

#include <imgui.h>

namespace EngineUI {

	extern std::map<std::string, bool> windowState;
	extern std::string currentSelectedItem;
	std::map<std::string, bool> loopUI(bool& applicationOpen, float &cameraSpeed, float &fov, bool& firstMouse);
	void initUI();
	void refreshGraphData();
}

#endif