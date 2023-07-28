#include "EngineUI.h"
#include "FileBrowser.h"
#include "Functions.h"
#include <iostream>
#include <imgui.h>
#include "implot.h"
#include <windows.h>
#include <algorithm> 
#include <string>
#include <stb_image.h>
#include <chrono>
#include <backends/imgui_impl_vulkan.h>
#include "Rendering.h"
#include "Debug.h"
#include "Serialization.h"
#include "Models.h"
#include "imguizmo.h"
#include "UnseenEngine.h"
#include "EngineUIFunctions.h"
#include <chrono>

namespace EngineUI {
   

    std::map<std::string, bool> windowState;

    bool statsWeekView = true;
    bool statsMonthView = false;
    bool statsYearView = false;

    //Robot vars

    //Blinking
    std::chrono::time_point<std::chrono::steady_clock> lastBlinkTime;
    bool robotOpenEyes = false;
    int robotBlinkFrames = 0;

    //Snoozing
    std::chrono::time_point<std::chrono::steady_clock> lastSnoreTime;
    bool isRobotSleeping = true;
    int robotSleepStage = 0;

    //Waving 
    std::chrono::time_point<std::chrono::steady_clock> lastWaveTime;
    bool isRobotWaving = false;
    int robotWaveStage = 0;
    

    void initUI() {
        windowState["showDemoWindow"] = true;
        windowState["showBlockList"] = true;
        windowState["showStats"] = true;
        windowState["showTimer"] = true;
        windowState["showTimer"] = true;

        debug.log("UI Initialised");

        ImGui::StyleColorsClassic();

    }

    std::map<std::string, bool> loopUI(bool& applicationOpen, float& cameraSpeed, float& fov, bool& firstMouse) {


        //Docking
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::Begin("My Docked Window", NULL, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
        ImGui::DockSpace(ImGui::GetID("MyDockSpace"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
        //Style
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowPadding = ImVec2(0.0f, 0.0f); //no padding
        style.Colors[ImGuiCol_WindowBg].w = 1.0f; //fully opaque windows
        style.WindowBorderSize = 0.0f;
        ImGui::End();


        //Demo window
        if (windowState["showDemoWindow"]) {
            ImGui::ShowDemoWindow(&windowState["showDemoWindow"]);
            ImPlot::ShowDemoWindow(&windowState["showDemoWindow"]);
        }


        //Blocklist
        if (windowState["showBlockList"]) {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(25, 15));
            ImGui::Begin("BlockList");
            ImGui::Text("Enter the URLs of the websites you would\nlike to be blocked whilst the app is open.");
            ImGui::Dummy(ImVec2(0, 10));
            ImGui::Text("Note: submitting a blocklist will reset your\ntimer. This is by design so you don't keep\nfiddling with it.");
            ImGui::Dummy(ImVec2(0, 10));
        
            static char url1[200] = "";
            static char url2[200] = "";
            static char url3[200] = "";
            static char url4[200] = "";
            static char url5[200] = ""; 
            static char url6[200] = "";
            static char url7[200] = ""; 
            static char url8[200] = "";
            static char url9[200] = "";
            static char url10[200] = "";
            float width = ImGui::GetContentRegionAvail().x;
            ImGui::PushItemWidth(width);
            ImGui::InputTextWithHint("##block1", "Enter url to block...", url1, IM_ARRAYSIZE(url1));
            ImGui::InputTextWithHint("##block2", "Enter url to block...", url2, IM_ARRAYSIZE(url2));
            ImGui::InputTextWithHint("##block3", "Enter url to block...", url3, IM_ARRAYSIZE(url3));
            ImGui::InputTextWithHint("##block4", "Enter url to block...", url4, IM_ARRAYSIZE(url4));
            ImGui::InputTextWithHint("##block5", "Enter url to block...", url5, IM_ARRAYSIZE(url5));
            ImGui::InputTextWithHint("##block6", "Enter url to block...", url6, IM_ARRAYSIZE(url6));
            ImGui::InputTextWithHint("##block7", "Enter url to block...", url7, IM_ARRAYSIZE(url7));
            ImGui::InputTextWithHint("##block8", "Enter url to block...", url8, IM_ARRAYSIZE(url8));
            ImGui::InputTextWithHint("##block9", "Enter url to block...", url9, IM_ARRAYSIZE(url9));
            ImGui::InputTextWithHint("##block10", "Enter url to block...", url10, IM_ARRAYSIZE(url10));
           
            ImGui::PopItemWidth();
            ImGui::Button("Submit Blocklist (Resets Timer)", ImVec2(600,70));
            ImGui::PopStyleVar();
            ImGui::End();
        }

        //Stats
        if (windowState["showStats"]) {
            ImGui::Begin("Stats");
            ImGui::Dummy(ImVec2(0, 15));

            // Add left padding
            ImGui::Dummy(ImVec2(10, 0));
            ImGui::SameLine();

            if (ImGui::Button("Week", ImVec2(200, 75))) {
                statsWeekView = true;
                statsMonthView = false;
                statsYearView = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Month", ImVec2(200, 75))) {
                statsWeekView = false;
                statsMonthView = true;
                statsYearView = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Year", ImVec2(200, 75))) {
                statsWeekView = false;
                statsMonthView = false;
                statsYearView = true;
            }

            ImGui::Dummy(ImVec2(0, 10));

            ImPlotStyle& style = ImPlot::GetStyle();

            ImVec4* colors = style.Colors;
            colors[ImPlotCol_Line] = IMPLOT_AUTO_COL;
            colors[ImPlotCol_Fill] = IMPLOT_AUTO_COL;
            colors[ImPlotCol_MarkerOutline] = IMPLOT_AUTO_COL;
            colors[ImPlotCol_MarkerFill] = IMPLOT_AUTO_COL;

            colors[ImPlotCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
            style.PlotPadding = ImVec2(25, 0);


            //Week Plot
            if (statsWeekView) {
                static const char* weekGlabels[] = { "Mon","Tue","Wed","Thu","Fri","Sat","Sun" };
                static const double weekPositions[] = { 0, 1,2,3,4,5,6 };
                static int weekGroups = 7;
                static ImS16 weekData[7] = { 5,3,2,5,3,0,0 };
                if (ImPlot::BeginPlot("This Week", ImVec2(-1,475))) {
                    ImPlot::SetupAxes("Day of Week", "Hours", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    ImPlot::SetupAxisTicks(ImAxis_X1, weekPositions, weekGroups, weekGlabels);
                    ImPlot::PlotBars("", weekData, 7, 0.7, 0);
                    ImPlot::EndPlot();
                }
            }

            //Month Plot
            if (statsMonthView) {
                static const char* monthGlabels[] = { "21","22","23","Current" };
                static const double monthPositions[] = { 0,1,2,3 };
                static int monthGroups = 4;
                static ImS16  monthData[4] = { 80,98,24,5 };
                if (ImPlot::BeginPlot("Last Four Weeks", ImVec2(-1, 475))) {
                    ImPlot::SetupAxes("Week Number", "Hours", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    ImPlot::SetupAxisTicks(ImAxis_X1, monthPositions, monthGroups, monthGlabels);
                    ImPlot::PlotBars("", monthData, 4, 0.5, 0);
                    ImPlot::EndPlot();
                }
            }


            //Year Plot
            if (statsYearView) {
                static const char* yearGlabels[] = { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };
                static const double yearPositions[] = { 0,1,2,3,4,5,6,7,8,9,10,11 };
                static int yearGroups = 12;
                static ImS16 yearData[12] = { 280,298,224,250,233,223,100,278,50,0,0,0 };
                if (ImPlot::BeginPlot("This Year", ImVec2(-1, 475))) {
                    ImPlot::SetupAxes("Month", "Hours", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    ImPlot::SetupAxisTicks(ImAxis_X1, yearPositions, yearGroups, yearGlabels);
                    ImPlot::PlotBars("", yearData, 12, 0.3, 0);
                    ImPlot::EndPlot();
                }
            }



            ImGui::End();
        }

        //Timer
        if (windowState["showTimer"]) {
            ImGui::Begin("Time");

            std::string robotHead;
            std::string robotBody;
            std::string robotFeet;

            //Robot blinking
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - lastBlinkTime).count() > 1) { // Change the number to adjust the blink speed
                robotOpenEyes = false;
                lastBlinkTime = now;
            }

            //Robot snoozing
            if (isRobotSleeping) {
                if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSnoreTime).count() > 500) { // Change the number to adjust the snore speed
                    lastSnoreTime = now;
                    robotSleepStage++;
                    if (robotSleepStage > 3)robotSleepStage = 0;
                }
            }

            //Robot waving
            if (isRobotWaving) {
                if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastWaveTime).count() > 500) { // Change the number to adjust the wave speed
                    lastWaveTime = now;
                    robotWaveStage++;
                    if(robotWaveStage > 1) robotWaveStage = 0;
                }
            }

            //Robot idle
            if (!isRobotSleeping && !isRobotWaving) robotHead = "[^_^]";
   
         
            //Robot Sleeping - animation
            if (isRobotSleeping) {

                switch (robotSleepStage) {
                case 0:
                    robotHead = "[-_-]";
                    break;
                case 1:
                    robotHead = "[-_-] Z";
                    break;
                case 2:
                    robotHead = "[-_-] Zz";
                    break;
                case 3:
                    robotHead = "[-_-] Zzz";
                    break;
                }
            }

            //Robot waving - animation
            if (isRobotWaving) {
                switch (robotWaveStage) {
                case 0:
                    robotHead = "\\[0_0]";
                    break;
                case 1:
                    robotHead = "[^_^]";
                    break;
                }
            }
            if (isRobotWaving) {
                robotBody = "(| |)>";
            }
            else {
                robotBody = "<(| |)>";
            }

            //Robot feet - animation
            robotFeet = "/ \\ ";

            //Assemble robot
            std::vector<std::string> robotParts = { robotHead, robotBody, robotFeet };

    
            //std::string timerContent = "       Welcome back, Jake!\nLet's get some serious work done.";
            //std::string timerContent = " You have been working for 36 minutes";
            std::string timerContent = " You have been idle for 45 minutes.\n       Have you been working?";

            // Invisible child window to enable centering for multiple elements
            ImGui::BeginChild("Centered child window", ImGui::GetContentRegionAvail(), false);

            // Get the window's size
            ImVec2 windowSize = ImGui::GetContentRegionAvail();

            // Calculate the width of the longest line of text
            float maxLineWidth = std::max<float>({ ImGui::CalcTextSize(robotHead.c_str()).x, ImGui::CalcTextSize(robotBody.c_str()).x, ImGui::CalcTextSize(robotFeet.c_str()).x, ImGui::CalcTextSize(timerContent.c_str()).x });

            // Calculate the total height of the content
            float totalContentHeight = ImGui::CalcTextSize((robotHead + "\n" + robotBody + "\n" + robotFeet).c_str()).y + ImGui::CalcTextSize(timerContent.c_str()).y;

            // Calculate the position to start drawing text to center it
            ImVec2 textPos = ImVec2((windowSize.x - maxLineWidth) * 0.5f, (windowSize.y - totalContentHeight) * 0.5f - 20.0f); // Subtract 20 to move everything up

            // Set the cursor to the calculated position and render the text
            for (const auto& part : robotParts) {
                ImVec2 partPos = ImVec2((windowSize.x - ImGui::CalcTextSize(part.c_str()).x) * 0.5f, textPos.y);
                ImGui::SetCursorPos(partPos);
                ImGui::Text(part.c_str());
                textPos.y += ImGui::CalcTextSize(part.c_str()).y;
            }

            // Add a small offset to the y-coordinate of the text position
            textPos.y += 30.0f;

            // Recalculate the position for the timerContent
            textPos.x = (windowSize.x - ImGui::CalcTextSize(timerContent.c_str()).x) * 0.5f;
            ImGui::SetCursorPos(textPos);
            ImGui::Text(timerContent.c_str());
          
            textPos.y += 180;
            textPos.x = (windowSize.x - 600) * 0.5f;
           
            ImGui::SetCursorPos(textPos);
         
            if (true) {
                ImGui::Button("Yes, keep\nidle time", ImVec2(296, 80));
                ImGui::SameLine();
                ImGui::Button("No, remove\nidle time", ImVec2(296, 80));
            }

            //Robot blink frames
            if (!robotOpenEyes)robotBlinkFrames++;
            if (!robotOpenEyes && robotBlinkFrames > 300) {
                robotOpenEyes = true;
                robotBlinkFrames = 0;
            }

            
            ImGui::EndChild();
            ImGui::End();
        }







        return windowState;
    }


}