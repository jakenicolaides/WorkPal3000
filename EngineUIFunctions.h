#pragma once
#include <glm.hpp>
#define NOMINMAX 
#include <string>
#include <gtc/quaternion.hpp>

namespace EngineUIFunctions {

	static glm::mat4 previousTransform;
	static glm::quat previousRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	;
	static int currentSelectedEntityId = -1;
	void savePreviousTransform();
	void entityInsepctor(std::string currentSelectedItem="", bool refreshStatics = false);
	void viewportCameraTranslation(bool update=false);
	void keyboardCommands();
	void grabbing();
	void scaling();
	void clearKeyboardCommands();
	void scaling();
	void updateTransformParameters(int entityId);
	void multiplyScale(glm::vec3 multiplyer);
	void saveKeyboardCommand();
	void rotating();
}