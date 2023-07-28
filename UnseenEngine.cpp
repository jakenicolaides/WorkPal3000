/*
Engine Setup Instructions:
 - Download and install Vulkan to the root of your c drive
 - Download the repo from perforce server
 - Link the p4ignore.txt file in the root to the system environment variables
 - Install and configure the P4VS plugin for visual studio

Naming conventions:

 - Structs, Classes, and Enumerated Types: These are named using CamelCase, starting with an uppercase letter. For example: struct EmployeeRecord
 - Member Functions and Variables: Member functions and variables in a class are named using camelCase, starting with a lowercase letter.
 - Global Variables: Global variables are named using camelCase, starting with a lowercase letter and with a suffix "_". For example: int globalVariable_;
 - Functions: Functions are named using camelCase, starting with a lowercase letter. For example: void processData();
 - Constants: Constants are named using all uppercase letters, with words separated by underscores. For example: const int MAX_RECORDS = 100;


*/


#include "UnseenEngine.h"
#include "Rendering.h"
#include "FileBrowser.h"
#include "Debug.h"
#include "Data.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "OpenXR.h"

int main() {

    Rendering::start();
    return 0;

}


namespace UnseenEngine {

    bool engineInitialised_ = false;

    void engineLoop() {

        if (Rendering::isPlayingGame && glfwGetKey(Rendering::window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
            Rendering::isPlayingGame = false;
        }

    }

    void initEngine() {

        engineInitialised_ = true;

    }

    void launchGame() {

        OpenXR::runOpenXR();
        Rendering::isPlayingGame = true;

    }

}



