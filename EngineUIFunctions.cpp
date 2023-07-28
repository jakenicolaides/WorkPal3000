#include "EngineUIFunctions.h"
#include "imgui.h"
#include "Functions.h"
#include "Data.h"
#include "Debug.h"
#include "EngineUI.h"
#include <algorithm>


namespace EngineUIFunctions {

    //Keyboard Navigation
    bool  isDoingShortcut  = false;
    bool isGrabbing = false;
    bool isGrabbingX = false;
    bool isGrabbingY = false;
    bool isGrabbingZ = false;
    bool isScaling = false;
    bool isScalingX = false; 
    bool isScalingY = false;
    bool isScalingZ = false;
    bool isRotating = false;
    bool isRotatingX = false;
    bool isRotatingY = false;
    bool isRotatingZ = false;
    double mousePrevX = 0;
    double mousePrevY = 0;
    int leftMouseFrames = 0;
    int rightMouseFrames = 0;
    int sKeyFrames = 0;
    int rKeyFrames = 0;

    //Transform params
    static glm::vec3 worldPosition;
    static glm::quat quatRotation;
    float currentScaleX ;
    float currentScaleY;
    float currentScaleZ;
    static glm::vec3 newScale;

    void updateTransformParameters(int entityId) {
        int entityIndex = Data::getEntityIndexFromId(currentSelectedEntityId);
        worldPosition = Functions::getWorldPosition(Data::entities.transform[entityIndex]);
        quatRotation = Data::entities.rotation[entityIndex];
        currentScaleX = glm::length(glm::vec3(Data::entities.transform[entityIndex][0]));
        currentScaleY = glm::length(glm::vec3(Data::entities.transform[entityIndex][1]));
        currentScaleZ = glm::length(glm::vec3(Data::entities.transform[entityIndex][2]));
        newScale = glm::vec3(currentScaleX, currentScaleY, currentScaleZ);
    }

	void entityInsepctor(std::string currentSelectedItem, bool refreshStatics) {

        //Getting the index of the entity
        currentSelectedEntityId = stoi(Functions::removeSubstring(currentSelectedItem, ".entity"));

        int entityIndex = Data::getEntityIndexFromId(currentSelectedEntityId);
        
        updateTransformParameters(currentSelectedEntityId);

        /*
        Translation
        */
        
        ImGui::Text("Pstn: ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        bool xPositionChanged = ImGui::DragFloat("##xposition", &worldPosition.x, 0.005f, -FLT_MAX, +FLT_MAX, "%.3f", ImGuiSliderFlags_None);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        bool yPositionChanged = ImGui::DragFloat("##yposition", &worldPosition.y, 0.005f, -FLT_MAX, +FLT_MAX, "%.3f", ImGuiSliderFlags_None);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        bool zPositionChanged = ImGui::DragFloat("##zposition", &worldPosition.z, 0.005f, -FLT_MAX, +FLT_MAX, "%.3f", ImGuiSliderFlags_None);

        if (xPositionChanged || yPositionChanged || zPositionChanged) {
            Data::entities.transform[entityIndex][3] = glm::vec4(worldPosition, 1.0f);
        }

        /*
            Rotation Euler
        */
      
        glm::vec3 eulerRotation = glm::degrees(glm::eulerAngles(quatRotation));
        ImGui::Text("RotE: ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        bool xRotationChanged = ImGui::DragFloat("##xEulerRotation", &eulerRotation.x, 0.05f, -FLT_MAX, +FLT_MAX, "%.3f", ImGuiSliderFlags_None);
        //if (isRotatingX) { ImGui::SetKeyboardFocusHere(-1); }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        bool yRotationChanged = ImGui::DragFloat("##yEulerRotation", &eulerRotation.y, 0.05f, -FLT_MAX, +FLT_MAX, "%.3f", ImGuiSliderFlags_None);
       // if (isRotatingY) { ImGui::SetKeyboardFocusHere(-1); }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        bool zRotationChanged = ImGui::DragFloat("##zEulerRotation", &eulerRotation.z, 0.05f, -FLT_MAX, +FLT_MAX, "%.3f", ImGuiSliderFlags_None);
        //if (isRotatingZ) { ImGui::SetKeyboardFocusHere(-1); }
        if (xRotationChanged || yRotationChanged || zRotationChanged) {
            Data::entities.rotation[entityIndex] = glm::quat(glm::radians(eulerRotation));
            quatRotation = glm::quat(glm::radians(eulerRotation));
        }

        /*
            Rotation Quaternion
        */
        ImGui::Text("RotQ: ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(110);

        bool xQuatRotationChanged = ImGui::DragFloat("##xQuatRotation", &quatRotation.x, 0.005f, -FLT_MAX, +FLT_MAX, "%.3f", ImGuiSliderFlags_None);
       
        ImGui::SameLine();
        ImGui::SetNextItemWidth(110);
        bool yQuatRotationChanged = ImGui::DragFloat("##yQuatRotation", &quatRotation.y, 0.005f, -FLT_MAX, +FLT_MAX, "%.3f", ImGuiSliderFlags_None);
        
        ImGui::SameLine();
        ImGui::SetNextItemWidth(110);
        bool zQuatRotationChanged = ImGui::DragFloat("##zQuatRotation", &quatRotation.z, 0.005f, -FLT_MAX, +FLT_MAX, "%.3f", ImGuiSliderFlags_None);
        
        ImGui::SameLine();
        ImGui::SetNextItemWidth(110);
        bool wQuatRotationChanged = ImGui::DragFloat("##wQuatRotation", &quatRotation.w, 0.005f, -1, +1, "%.3f", ImGuiSliderFlags_None);

        if (xQuatRotationChanged || yQuatRotationChanged || zQuatRotationChanged) {
            quatRotation = glm::normalize(quatRotation);
            Data::entities.rotation[entityIndex] = quatRotation;
        }


        /*
        Scale        
        */

        //Normalize the basis vectors of the matrix
        glm::mat4 normalizedMatrix = Data::entities.transform[entityIndex];
        normalizedMatrix[0] /= currentScaleX;
        normalizedMatrix[1] /= currentScaleY;
        normalizedMatrix[2] /= currentScaleZ;

        
        const float minimumScale = 0.00001f;

        ImGui::Text("Scle: ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        bool xScaleChanged = ImGui::DragFloat("##xscale", &newScale.x, 0.005f, minimumScale, +FLT_MAX, "%.3f", ImGuiSliderFlags_None);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        bool yScaleChanged = ImGui::DragFloat("##yscale", &newScale.y, 0.005f, minimumScale, +FLT_MAX, "%.3f", ImGuiSliderFlags_None);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        bool zScaleChanged = ImGui::DragFloat("##zscale", &newScale.z, 0.005f, minimumScale, +FLT_MAX, "%.3f", ImGuiSliderFlags_None);

        //Uniform scaling option
        bool uPressed = (glfwGetKey(Rendering::window, GLFW_KEY_U) == GLFW_PRESS);
        if (uPressed && (xScaleChanged || yScaleChanged || zScaleChanged)) {
            static glm::vec3 prevScale = newScale;
            glm::vec3 deltaScale = newScale - prevScale;

            // Find the first non-zero delta value
            float uniformDelta = 0.0f;
            for (float delta : {deltaScale.x, deltaScale.y, deltaScale.z}) {
                if (std::abs(delta) > 1e-6) {
                    uniformDelta = delta;
                    break;
                }
            }

            newScale = prevScale + glm::vec3(uniformDelta, uniformDelta, uniformDelta);

            for (int i = 0; i < 3; ++i) {
                newScale[i] = std::max(newScale[i], minimumScale);
            }

            prevScale = newScale;
        }

        if (xScaleChanged || yScaleChanged || zScaleChanged) {
            //Apply the new scale to the matrix
			Data::entities.transform[entityIndex] = glm::scale(normalizedMatrix, newScale);
		}


        
	}

    void viewportCameraTranslation(bool update) {
        
        static glm::vec3 cameraPosition = Rendering::cameraPosition;
        if (update) { cameraPosition = Rendering::cameraPosition; return; } 
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        bool xPositionChanged = ImGui::DragFloat("##xposition", &cameraPosition.x, 0.005f, -FLT_MAX, +FLT_MAX, "%.1f", ImGuiSliderFlags_None);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        bool yPositionChanged = ImGui::DragFloat("##yposition", &cameraPosition.y, 0.005f, -FLT_MAX, +FLT_MAX, "%.1f", ImGuiSliderFlags_None);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        bool zPositionChanged = ImGui::DragFloat("##zposition", &cameraPosition.z, 0.005f, -FLT_MAX, +FLT_MAX, "%.1f", ImGuiSliderFlags_None);

        if (xPositionChanged || yPositionChanged || zPositionChanged) {
            Rendering::cameraPosition = cameraPosition;
        }
    }

    /* keyboard commands */

    void clearKeyboardCommands() {
        isDoingShortcut = false;
        isGrabbing = false;
        isGrabbingX = false;
        isGrabbingY = false;
        isGrabbingZ = false;
        isScaling = false;
        isScalingX = false;
        isScalingY = false;
        isScalingZ = false;
        isRotating = false;
        isRotatingX = false;
        isRotatingY = false;
        isRotatingZ = false;
    }

    void keyboardCommands() {

        if (currentSelectedEntityId == -1) { return; }

        //Remove mouse when doing these commands
        ImGuiIO& io = ImGui::GetIO();
        if (isDoingShortcut) {
            io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
        }
        else {
            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        }

        // Check if the left mouse button is pressed
        if (glfwGetMouseButton(Rendering::window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && rightMouseFrames == 0 && isDoingShortcut == true)
        {
            saveKeyboardCommand();
        }
        // Check if the right mouse button is pressed
        if (glfwGetMouseButton(Rendering::window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && rightMouseFrames == 0 && isDoingShortcut == true)
        {
            int entityIndex = Data::getEntityIndexFromId(currentSelectedEntityId);

            if (isRotating) {
                Data::entities.rotation[entityIndex] = previousRotation;
            }


            Data::entities.transform[entityIndex] = previousTransform;
            updateTransformParameters(Data::entities.id[entityIndex]);

            clearKeyboardCommands();
            debug.log("Transform cancelled");

        }

        //Grabbing

        if (glfwGetKey(Rendering::window, GLFW_KEY_G) == GLFW_PRESS && isDoingShortcut == false)
        {
            isDoingShortcut = true;
            isGrabbing = true;
            savePreviousTransform();

            debug.log("Grabbing: select an axis");
        }

        if (glfwGetKey(Rendering::window, GLFW_KEY_X) == GLFW_PRESS && isGrabbing && isGrabbingX == false)
        {

            isGrabbingX = true;
            isGrabbingY = false;
            isGrabbingZ = false;
            glfwGetCursorPos(Rendering::window, &mousePrevX, &mousePrevY);
            debug.log("Grabbing in the X axis");
        }

        if (glfwGetKey(Rendering::window, GLFW_KEY_Y) == GLFW_PRESS && isGrabbing && isGrabbingZ == false)
        {
            isGrabbingX = false;
            isGrabbingY = false;
            isGrabbingZ = true;

            glfwGetCursorPos(Rendering::window, &mousePrevX, &mousePrevY);
            debug.log("Grabbing in the Z axis");
        }

        if (glfwGetKey(Rendering::window, GLFW_KEY_Z) == GLFW_PRESS && isGrabbing && isGrabbingY == false)
        {
            isGrabbingX = false;
            isGrabbingY = true;
            isGrabbingZ = false;
            glfwGetCursorPos(Rendering::window, &mousePrevX, &mousePrevY);
            debug.log("Grabbing in the Y axis");
        }

        if (isGrabbingX || isGrabbingY || isGrabbingZ) { grabbing(); }

        // Rotation
        if (glfwGetKey(Rendering::window, GLFW_KEY_R) == GLFW_PRESS && glfwGetMouseButton(Rendering::window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS && sKeyFrames < 5 && isDoingShortcut == false)
        {
            glfwGetCursorPos(Rendering::window, &mousePrevX, &mousePrevY);
            savePreviousTransform();
            isDoingShortcut = true;
            isRotating = true;
            debug.log("Rotating: select an axis...");
        }

        if (glfwGetKey(Rendering::window, GLFW_KEY_X) == GLFW_PRESS && isRotating && isRotatingX == false)
        {

            isRotatingX = true;
            isRotatingY = false;
            isRotatingZ = false;
            glfwGetCursorPos(Rendering::window, &mousePrevX, &mousePrevY);
            debug.log("Rotating in the X axis");
        }

        if (glfwGetKey(Rendering::window, GLFW_KEY_Y) == GLFW_PRESS && isRotating && isRotatingZ == false)
        {
            isRotatingX = false;
            isRotatingY = false;
            isRotatingZ = true;
            glfwGetCursorPos(Rendering::window, &mousePrevX, &mousePrevY);
            debug.log("Rotating in the Z axis");
        }

        if (glfwGetKey(Rendering::window, GLFW_KEY_Z) == GLFW_PRESS && isRotating && isRotatingY == false)
        {
            isRotatingX = false;
            isRotatingY = true;
            isRotatingZ = false;
            debug.log("Rotating in the Y axis");
            glfwGetCursorPos(Rendering::window, &mousePrevX, &mousePrevY);
        }

        if ((isRotatingX || isRotatingY || isRotatingZ)) {
            rotating();
        }

        //Scaling
        if (glfwGetKey(Rendering::window, GLFW_KEY_S) == GLFW_PRESS && glfwGetMouseButton(Rendering::window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS && sKeyFrames < 5 && isDoingShortcut == false)
        {
            glfwGetCursorPos(Rendering::window, &mousePrevX, &mousePrevY);
            savePreviousTransform();
            isDoingShortcut = true;
            isScaling = true;
            debug.log("Scaling uniformly: scale in axis X Y or Z");
        }

        if (glfwGetKey(Rendering::window, GLFW_KEY_X) == GLFW_PRESS && isScaling && isScalingX == false)
        {

            isScalingX = true;
            isScalingY = false;
            isScalingZ = false;
            glfwGetCursorPos(Rendering::window, &mousePrevX, &mousePrevY);
            debug.log("Scaling in the X axis");
        }

        if (glfwGetKey(Rendering::window, GLFW_KEY_Y) == GLFW_PRESS && isScaling && isScalingZ == false)
        {

            isScalingX = false;
            isScalingY = false;
            isScalingZ = true;
            glfwGetCursorPos(Rendering::window, &mousePrevX, &mousePrevY);
            debug.log("Scaling in the Z axis");
        }

        if (glfwGetKey(Rendering::window, GLFW_KEY_Z) == GLFW_PRESS && isScaling && isScalingY == false)
        {
            isScalingX = false;
            isScalingY = true;
            isScalingZ = false;
            debug.log("Scaling in the Y axis");
            glfwGetCursorPos(Rendering::window, &mousePrevX, &mousePrevY);
        }

        if (isScaling) {
            for (int i = 1; i <= 9; i++) {
                if (glfwGetKey(Rendering::window, GLFW_KEY_0 + i) == GLFW_PRESS) {
                    if (isScalingX) {
                        multiplyScale(glm::vec3(i, 1, 1));
                    }
                    else if (isScalingY) {
                        multiplyScale(glm::vec3(1, i, 1));
                    }
                    else if (isScalingZ) {
                        multiplyScale(glm::vec3(1, 1, i));
                    }

                }
            }
        }

        if (isScaling) {

            scaling();
        }

        //Counting frames of key presses
        if (glfwGetKey(Rendering::window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) { leftMouseFrames++; }
        if (glfwGetKey(Rendering::window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) { leftMouseFrames = 0; }
        if (glfwGetKey(Rendering::window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) { rightMouseFrames++; }
        if (glfwGetKey(Rendering::window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) { rightMouseFrames = 0; }
        if (glfwGetKey(Rendering::window, GLFW_KEY_S) == GLFW_PRESS) { sKeyFrames++; }
        if (glfwGetKey(Rendering::window, GLFW_KEY_S) == GLFW_RELEASE) { sKeyFrames = 0; }


      
    }

    void grabbing() {
        double mouseX = 0;
        double mouseY = 0;

        glfwGetCursorPos(Rendering::window, &mouseX, &mouseY);

        double deltaX = mouseX - mousePrevX;
        double deltaY = mouseY - mousePrevY;

        int entityIndex = Data::getEntityIndexFromId(currentSelectedEntityId);

        // Get the camera's view and projection matrices
        glm::mat4 viewMatrix = glm::lookAtRH(Rendering::cameraPosition, Rendering::cameraPosition + Rendering::cameraFront, Rendering::cameraUp);
        glm::mat4 projectionMatrix = glm::perspective(glm::radians(Rendering::fov), Rendering::swapChainExtent.width / (float)Rendering::swapChainExtent.height, 0.1f, 10000.0f);

        // Get window size
        int windowWidth, windowHeight;
        glfwGetWindowSize(Rendering::window, &windowWidth, &windowHeight);

        // Calculate the NDC (Normalized Device Coordinates)
        glm::vec2 ndcMovement = glm::vec2((float)deltaX / (float)windowWidth * 2.0f, -(float)deltaY / (float)windowHeight * 2.0f);

        // Unproject the NDC movement to world space
        glm::vec4 startNDC = glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
        glm::vec4 endNDC = startNDC + glm::vec4(ndcMovement, 0.0f, 0.0f);

        glm::vec4 startWorld = glm::inverse(projectionMatrix * viewMatrix) * startNDC;
        glm::vec4 endWorld = glm::inverse(projectionMatrix * viewMatrix) * endNDC;

        startWorld /= startWorld.w;
        endWorld /= endWorld.w;

        glm::vec3 worldMovement = glm::vec3(endWorld - startWorld);

        glm::vec3 movementVector;
        const float movementMultiplyer = 20.0f;

        //Get the vector between the camera and the model
        glm::vec3 distaceVector = glm::vec3(glm::inverse(Data::entities.transform[entityIndex]) * glm::vec4(Rendering::cameraPosition, 1.0f));
        float distance = glm::length(distaceVector);

        if (isGrabbingX) {
            movementVector = glm::vec3(worldMovement.x, 0.0f, 0.0f);
        }
        else if (isGrabbingY) {
            movementVector = glm::vec3(0.0f, worldMovement.y, 0.0f);
            movementVector = glm::vec3(0.0f, worldMovement.y, 0.0f);
        }
        else {
            movementVector = glm::vec3(0.0f, 0.0f, worldMovement.z);
        }

        //scale should also affect speed
        float currentScaleX = glm::length(glm::vec3(Data::entities.transform[entityIndex][0]));
        float currentScaleY = glm::length(glm::vec3(Data::entities.transform[entityIndex][1]));
        float currentScaleZ = glm::length(glm::vec3(Data::entities.transform[entityIndex][2]));

        glm::mat4 normalizedMatrix = Data::entities.transform[entityIndex];
        normalizedMatrix[0] /= currentScaleX;
        normalizedMatrix[1] /= currentScaleY;
        normalizedMatrix[2] /= currentScaleZ;

        float scaleUni = (currentScaleX + currentScaleY + currentScaleZ) / 3;

        glm::mat4 translation = glm::translate(glm::mat4(1.0f), movementVector * distance * movementMultiplyer * scaleUni);
        Data::entities.transform[entityIndex] = translation * Data::entities.transform[entityIndex];

        glfwGetCursorPos(Rendering::window, &mousePrevX, &mousePrevY);
    }

    void rotating() {

        int entityIndex = Data::getEntityIndexFromId(currentSelectedEntityId);

        double mouseX = 0;
        double mouseY = 0;

        glfwGetCursorPos(Rendering::window, &mouseX, &mouseY);

        double deltaY = mouseY - mousePrevY;

        float angle = glm::radians(deltaY);
        glm::vec3 axis = glm::vec3(0, 0, 0);
        if (isRotatingX) {
            axis = glm::vec3(1, 0, 0);
        }
        else if (isRotatingY) {
            axis = glm::vec3(0, 1, 0);
        }
        else {
            axis = glm::vec3(0, 0, 1);
        }

       

        glm::quat newRotation = glm::angleAxis(angle, axis);
        
        glm::quat updatedRotation = newRotation * Data::entities.rotation[entityIndex];
        Data::entities.rotation[entityIndex] = updatedRotation;

        glfwGetCursorPos(Rendering::window, &mousePrevX, &mousePrevY);
    }

    void scaling() {

        const float minimumScale = 0.00001f;


        int entityIndex = Data::getEntityIndexFromId(currentSelectedEntityId);

        double mouseX = 0;
        double mouseY = 0;
       
        glfwGetCursorPos(Rendering::window, &mouseX, &mouseY);

        double deltaX = mouseX - mousePrevX;
        double deltaY = mouseY - mousePrevY;
        double deltaBoth = (deltaX + deltaY) / 2;

        float currentScaleX = glm::length(glm::vec3(Data::entities.transform[entityIndex][0]));
        float currentScaleY = glm::length(glm::vec3(Data::entities.transform[entityIndex][1]));
        float currentScaleZ = glm::length(glm::vec3(Data::entities.transform[entityIndex][2]));

        //Normalize the basis vectors of the matrix
        glm::mat4 normalizedMatrix = Data::entities.transform[entityIndex];
        normalizedMatrix[0] /= currentScaleX;
        normalizedMatrix[1] /= currentScaleY;
        normalizedMatrix[2] /= currentScaleZ;

        glm::vec3 newScale;
        if (isScalingX) {
            newScale = glm::vec3(currentScaleX - ( deltaBoth / 100), currentScaleY , currentScaleZ);
        }
        else if (isScalingY) {
            newScale = glm::vec3(currentScaleX, currentScaleY - ( deltaBoth / 100), currentScaleZ);
        }
        else if (isScalingZ) {
            newScale = glm::vec3(currentScaleX, currentScaleY, currentScaleZ - ( deltaBoth / 100));
        }
        else {
            newScale = glm::vec3(currentScaleX - ( deltaBoth / 100), currentScaleY - ( deltaBoth / 100), currentScaleZ - ( deltaBoth / 100));
        }

        for (int i = 0; i < 3; ++i) {
            newScale[i] = std::max(newScale[i], minimumScale);
        }

        Data::entities.transform[entityIndex] = glm::scale(normalizedMatrix, newScale);
        
        glfwGetCursorPos(Rendering::window, &mousePrevX, &mousePrevY);
      

    }

   
    void multiplyScale(glm::vec3 multiplyer) {

        const float minimumScale = 0.00001f;

        int entityIndex = Data::getEntityIndexFromId(currentSelectedEntityId);

        float currentScaleX = glm::length(glm::vec3(Data::entities.transform[entityIndex][0]));
        float currentScaleY = glm::length(glm::vec3(Data::entities.transform[entityIndex][1]));
        float currentScaleZ = glm::length(glm::vec3(Data::entities.transform[entityIndex][2]));

        //Normalize the basis vectors of the matrix
        glm::mat4 normalizedMatrix = Data::entities.transform[entityIndex];
        normalizedMatrix[0] /= currentScaleX;
        normalizedMatrix[1] /= currentScaleY;
        normalizedMatrix[2] /= currentScaleZ;

       
        glm::vec3 newScale = glm::vec3(multiplyer.x * currentScaleX, multiplyer.y * currentScaleY, multiplyer.z * currentScaleZ);

        for (int i = 0; i < 3; ++i) {
            newScale[i] = std::max(newScale[i], minimumScale);
        }

        Data::entities.transform[entityIndex] = glm::scale(normalizedMatrix, newScale);
        saveKeyboardCommand();
    }

    void savePreviousTransform() {
        int entityIndex = 0;
        for (int i = 0; i < Data::numEntities; ++i) {
            if (Data::entities.id[i] == currentSelectedEntityId) {
                entityIndex = i;
                break;
            }
        }

        previousTransform = Data::entities.transform[entityIndex];
        previousRotation = Data::entities.rotation[entityIndex];
    }

    void saveKeyboardCommand() {
        updateTransformParameters(currentSelectedEntityId);
        clearKeyboardCommands();
        debug.log("Transform saved");
    }

  

}