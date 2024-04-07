#include "EngineUI.h"
#include <imgui.h>
#include <chrono>
#include "implot.h"
#include "Rendering.h"
#include "WorkPal3000.h"

namespace EngineUI {
   

    std::map<std::string, bool> windowState;

    bool statsWeekView = true;
    bool statsMonthView = false;
    bool statsYearView = false;
    std::vector<std::string> datesInWeek;
    static double weekData[7] = { 0,0,0,0,0,0,0 };
    static double monthData[4] = {0,0,0,0};
    static double yearData[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
    char emailBuffer[200] = { 0 };
    char passwordBuffer[200] = {0};
    nlohmann::json userData;


    //Robot vars

    //Idle/Blinking
    std::chrono::time_point<std::chrono::steady_clock> lastBlinkTime;
    bool isRobotIdle = true;
    bool robotOpenEyes = true;

    //Snoozing
    std::chrono::time_point<std::chrono::steady_clock> lastSnoreTime;
    bool isRobotSleeping = false;
    int robotSleepStage = 0;

    //Waving 
    std::chrono::time_point<std::chrono::steady_clock> lastWaveTime;
    bool isRobotWaving = false;
    int robotWaveStage = 0;

    static int soundFileIndex = 0;
    const char* soundFileNames[] = { "none", "white-noise.wav", "brown-noise.wav", "rainy-window.wav", "ocean-waves.mp3", "cafe.mp3", "woodland.wav", "air-conditioner.wav" };

    

    void initUI() {

        if (WorkPal3000::needsOneTimeSetup) {
            windowState["showSetup"] = true;
        }
        else if (!WorkPal3000::subscriptionActive) {
            windowState["showSubscription"] = true;
        }else{
            windowState["showDemoWindow"] = false;
            windowState["showBlockList"] = true;
            windowState["showStats"] = true;
            windowState["showSettings"] = true;
            windowState["showTimer"] = true;
            windowState["showSetup"] = false;
        }
        
        ImGui::StyleColorsClassic();
        
        // Get a reference to the current ImGui style
        ImGuiStyle& style = ImGui::GetStyle();

        std::ifstream inFile("userdata.json");
        if (inFile.is_open()) {
            inFile >> userData;
            if (userData.contains("ambientSoundFile")) {
                size_t length = sizeof(soundFileNames) / sizeof(soundFileNames[0]);
                for (size_t i = 0; i < length; i++) {
                    std::string thisSoundFile = soundFileNames[i];
                    if (thisSoundFile == userData["ambientSoundFile"].get<std::string>()) soundFileIndex = i;
                }
            }
        }

        // Change the color for the window background
        // The parameters for ImVec4 are (r, g, b, a) -- all between 0.0f and 1.0f
        // style.Colors[ImGuiCol_WindowBg] = ImVec4(0.882f, 0.69f, 1.0f, 1.0f);
        //style.Colors[ImGuiCol_Text] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);  //

    }

    void refreshGraphData() {

        double totalHoursThisWeek = 0;
        double totalHoursLastWeek = 0;
        double totalHoursTwoWeeksAgo = 0;
        double totalHoursThreeWeeksAgo = 0;
        auto datesInWeek = WorkPal3000::getDatesInWeek(WorkPal3000::currentWeekOfYear());
        auto datesLastWeek = WorkPal3000::getDatesInWeek(WorkPal3000::currentWeekOfYear()-1);
        auto datesTwoWeeksAgo = WorkPal3000::getDatesInWeek(WorkPal3000::currentWeekOfYear()-2);
        auto datesThreeWeeksAgo = WorkPal3000::getDatesInWeek(WorkPal3000::currentWeekOfYear()-3);


        //Week Data
        for (size_t i = 0; i < datesInWeek.size(); i++)
        {
            double secondsOnDate = WorkPal3000::getSecondsOnDate(datesInWeek[i]);
            double hoursOnDate = secondsOnDate /3600;
            totalHoursThisWeek += hoursOnDate;
            weekData[i] = hoursOnDate;
        }

        //Month Data 
        for (size_t i = 0; i < datesLastWeek.size(); i++) { //-1 Week
            double secondsOnDate = WorkPal3000::getSecondsOnDate(datesLastWeek[i]);
            double hoursOnDate = secondsOnDate / 3600;
            totalHoursLastWeek += hoursOnDate;
        }
        for (size_t i = 0; i < datesLastWeek.size(); i++) { //-2 Week
            double secondsOnDate = WorkPal3000::getSecondsOnDate(datesTwoWeeksAgo[i]);
            double hoursOnDate = secondsOnDate / 3600;
            totalHoursTwoWeeksAgo += hoursOnDate;
        }
        for (size_t i = 0; i < datesLastWeek.size(); i++) { //-3 Week
            double secondsOnDate = WorkPal3000::getSecondsOnDate(datesThreeWeeksAgo[i]);
            double hoursOnDate = secondsOnDate / 3600;
            totalHoursThreeWeeksAgo += hoursOnDate;
        }

        monthData[3] = totalHoursThisWeek;
        monthData[2] = totalHoursLastWeek;
        monthData[1] = totalHoursTwoWeeksAgo;
        monthData[0] = totalHoursThreeWeeksAgo;

        //Year data
        WorkPal3000::getHoursInYear(yearData);
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
            ImGui::Begin("BlockList", 0, ImGuiWindowFlags_NoMove);
            ImGui::Text("Enter the URLs of the websites you would\nlike to be blocked whilst the app is open.");
            ImGui::Dummy(ImVec2(0, 10));
            ImGui::Text("Note: submitting a blocklist will reset your\ntimer. This is by design so you don't keep\nfiddling with it.");
            ImGui::Dummy(ImVec2(0, 10));

            float width = ImGui::GetContentRegionAvail().x;
            ImGui::PushItemWidth(width);

            for (size_t i = 0; i < 10; i++) {
                char buffer[200];
                buffer[0] = '\0';
                std::string id = "##block" + std::to_string(i);

                if (WorkPal3000::blocklist.size() > i) {
                    strcpy_s(buffer, WorkPal3000::blocklist[i].c_str());
                }

                ImGui::InputTextWithHint(id.c_str(), "Enter url to block...", buffer, IM_ARRAYSIZE(buffer));

                if (WorkPal3000::blocklist.size() > i) {
                    WorkPal3000::blocklist[i] = std::string(buffer);
                }
                else {
                    // if i is not a valid index in blocklist, add new string to the blocklist
                    WorkPal3000::blocklist.push_back(std::string(buffer));
                }
            }


            ImGui::PopItemWidth();
            if (ImGui::Button("Submit Blocklist (Resets Timer)", ImVec2(600, 70))) {
                std::vector<std::string> urls = WorkPal3000::blocklist;
                WorkPal3000::submitBlocklist(urls);
            }
            ImGui::PopStyleVar();
            ImGui::End();
        }

        //Stats
        if (windowState["showStats"]) {

            ImGui::Begin("Stats", 0,  ImGuiWindowFlags_NoMove);
            if (ImGui::IsWindowAppearing()) refreshGraphData();
            

            ImGui::Dummy(ImVec2(0, 15));

            // Add left padding
            ImGui::Dummy(ImVec2(10, 0));
            ImGui::SameLine();

            if (ImGui::Button("Week", ImVec2(200, 75))) {
                statsWeekView = true;
                statsMonthView = false;
                statsYearView = false;
                refreshGraphData();
            }
            ImGui::SameLine();
            if (ImGui::Button("Month", ImVec2(200, 75))) {
                statsWeekView = false;
                statsMonthView = true;
                statsYearView = false;
                refreshGraphData();
            }
            ImGui::SameLine();
            if (ImGui::Button("Year", ImVec2(200, 75))) {
                statsWeekView = false;
                statsMonthView = false;
                statsYearView = true;
                refreshGraphData();
            }

            ImGui::Dummy(ImVec2(0, 10));

            ImPlotStyle& style = ImPlot::GetStyle();

            ImVec4* colors = style.Colors;
            colors[ImPlotCol_Line] = IMPLOT_AUTO_COL;
            colors[ImPlotCol_Fill] = ImVec4(1.0f, 0.843f, 0.0f, 1.0f);
           ;
            colors[ImPlotCol_MarkerOutline] = IMPLOT_AUTO_COL;
            colors[ImPlotCol_MarkerFill] = IMPLOT_AUTO_COL;

            colors[ImPlotCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
            style.PlotPadding = ImVec2(25, 0);


           
           
            if (statsWeekView) {
                static const char* weekGlabels[] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
                static const double weekPositions[] = { 0, 1, 2, 3, 4, 5, 6 };
                static int weekGroups = 7;

                if (ImPlot::BeginPlot("This Week", ImVec2(-1, 475))) {
                    ImPlot::SetupAxes("Day of Week", "Hours", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    ImPlot::SetupAxisTicks(ImAxis_X1, weekPositions, weekGroups, weekGlabels);

                    // Plot each bar individually to set custom colors
                    for (int i = 0; i < weekGroups; i++) {
                        ImVec4 barColor = ImVec4(1.0, 1.0, 1.0, 1.0); // Default color

                        // Assign color based on hours
                        if (weekData[i] >= 13.0) barColor = ImVec4(0.576, 0.882, 1.0, 1.0); // Platinum
                        else if (weekData[i] >= 12.0) barColor = ImVec4(1.0, 0.816, 0.137, 1.0); // Gold
                        else if (weekData[i] >= 10.0) barColor = ImVec4(0.75, 0.75, 0.75, 1.0); // Silver
                        else if (weekData[i] >= 8.0) barColor = ImVec4(0.6, 0.4, 0.2, 1.0); // Brown 

                        ImPlot::PushStyleColor(ImPlotCol_Fill, barColor);
                        ImPlot::PlotBars("", &weekData[i], 1, 0.7, i);
                        ImPlot::PopStyleColor();
                    }

                    ImPlot::EndPlot();
                }
            }

            //Month Plot
            if (statsMonthView) {
                static const char* monthGlabels[] = { "-3", "-2 ", "-1", "Current" };
                static const double monthPositions[] = { 0, 1, 2, 3 };
                static int monthGroups = 4;

                if (ImPlot::BeginPlot("Last Four Weeks", ImVec2(-1, 475))) {
                    ImPlot::SetupAxes("Week Number", "Hours", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    ImPlot::SetupAxisTicks(ImAxis_X1, monthPositions, monthGroups, monthGlabels);

                    // Plot each bar individually to set custom colors
                    for (int i = 0; i < monthGroups; i++) {
                        ImVec4 barColor = ImVec4(1.0, 1.0, 1.0, 1.0); // Default color

                        // Assign color based on hours
                        if (monthData[i] >= 13.0 * 5.0) barColor = ImVec4(0.9, 0.9, 0.9, 1.0); // Platinum, assuming 13 hours per day over a week
                        else if (monthData[i] >= 12.0 * 5.0) barColor = ImVec4(1.0, 0.843, 0.0, 1.0); // Gold, assuming 12 hours per day over a week
                        else if (monthData[i] >= 10.0 * 5.0) barColor = ImVec4(0.75, 0.75, 0.75, 1.0); // Silver, assuming 10 hours per day over a week
                        else if (monthData[i] >= 8.0 * 5.0) barColor = ImVec4(0.6, 0.4, 0.2, 1.0); // Brown, assuming 8 hours per day over a week

                        ImPlot::PushStyleColor(ImPlotCol_Fill, barColor);
                        ImPlot::PlotBars("", &monthData[i], 1, 0.5, i); // Note the width of 0.5 to match the original month view settings
                        ImPlot::PopStyleColor();
                    }

                    ImPlot::EndPlot();
                }
            }



            //Year Plot
            if (statsYearView) {
                static const char* yearGlabels[] = { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };
                static const double yearPositions[] = { 0,1,2,3,4,5,6,7,8,9,10,11 };
                static int yearGroups = 12;
                if (ImPlot::BeginPlot("This Year", ImVec2(-1, 475))) {
                    ImPlot::SetupAxes("Month", "Hours", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    ImPlot::SetupAxisTicks(ImAxis_X1, yearPositions, yearGroups, yearGlabels);
                    ImPlot::PlotBars("", yearData, 12, 0.3, 0);
                    ImPlot::EndPlot();
                }
            }



            ImGui::End();
        }

        if (windowState["showSettings"]) {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
            ImGui::Begin("Settings", 0, ImGuiWindowFlags_NoMove);

            // Using the _simplified_ one-liner Combo() api here
           // See "Combo" section for examples of how to use the more flexible BeginCombo()/EndCombo() api.
           
            std::string filename = soundFileNames[soundFileIndex];
            if (filename != WorkPal3000::ambientSoundFile) {
                WorkPal3000::ambientSoundMutex.lock();
                WorkPal3000::ambientSoundFile = filename;
                WorkPal3000::ambientSoundMutex.unlock();
                std::cout << "Changing the wav" << std::endl;
                //save changes
                std::ifstream inFile("userdata.json");
                if (inFile.is_open()) {
                    inFile >> userData;
                    userData["ambientSoundFile"] = WorkPal3000::ambientSoundFile;
                }
                std::ofstream outFile("userdata.json");
                if (outFile.is_open()) outFile << userData;
            }
            ImGui::SetNextItemWidth(200);
            ImGui::Dummy(ImVec2(0, 10));
            ImGui::Combo("Ambient Sound", &soundFileIndex, soundFileNames, IM_ARRAYSIZE(soundFileNames));
            ImGui::Dummy(ImVec2(0, 10));
            ImGui::SetNextItemWidth(200);
            if (ImGui::InputInt("Idle Timer (minutes)", &WorkPal3000::idleDuration)) {
                // If the value was modified, we clamp it
                WorkPal3000::idleDuration = std::clamp(WorkPal3000::idleDuration, 1, 1440);
                //save changes
                std::ifstream inFile("userdata.json");
                if (inFile.is_open()) {
                    inFile >> userData;
                    userData["idleDuration"] = WorkPal3000::idleDuration;
                }
                std::ofstream outFile("userdata.json");
                if (outFile.is_open()) outFile << userData;
            }
            ImGui::Dummy(ImVec2(0, 10));
            if (ImGui::Checkbox("Play Interval Sounds", &WorkPal3000::playIntervalSounds)) {

                //save changes
                std::ifstream inFile("userdata.json");
                if (inFile.is_open()) {
                    inFile >> userData;
                    userData["playIntervalSounds"] = WorkPal3000::playIntervalSounds;
                }
                std::ofstream outFile("userdata.json");
                if (outFile.is_open()) outFile << userData;
            }
            ImGui::Dummy(ImVec2(0, 10));
            if (WorkPal3000::playIntervalSounds) {
                ImGui::SetNextItemWidth(200);
                if (ImGui::InputInt("Interval Timer (minutes) ", &WorkPal3000::intervalDuration)) {
                    // If the value was modified, we clamp it
                    WorkPal3000::intervalDuration = std::clamp(WorkPal3000::intervalDuration, 1, 1440);

                    //save changes
                    std::ifstream inFile("userdata.json");
                    if (inFile.is_open()) {
                        inFile >> userData;
                        userData["intervalDuration"] = WorkPal3000::intervalDuration;
                    }
                    std::ofstream outFile("userdata.json");
                    if (outFile.is_open()) outFile << userData;

                }
            }

            float remainingSpace = ImGui::GetContentRegionAvail().y;
            std::string versionId = "WorkPal3000 V" + WorkPal3000::version;
            
            // Calculate the space you need for the controls you want at the bottom
            float knownContentHeight =
                ImGui::CalcTextSize(versionId.c_str()).y +
                ImGui::CalcTextSize("For bugs, support or anything else:").y +
                ImGui::GetTextLineHeight() +  // Assuming InputText height is roughly a single line of text
                20 + 5; // Adding space from the Dummies
            float dummyHeight = remainingSpace - knownContentHeight;
            ImGui::Dummy(ImVec2(0, dummyHeight - 30));


           
            ImGui::Text(versionId.c_str());
            ImGui::Dummy(ImVec2(0, 20));
            char buf[256] = "support@workpal3000.com";
            ImGui::Text("For bugs, support or anything else:");
            ImGui::Dummy(ImVec2(0, 5));
            ImGui::SetNextItemWidth(320); // set width of next item
            ImGui::InputText("##email", buf, IM_ARRAYSIZE(buf), ImGuiInputTextFlags_ReadOnly);


            ImGui::PopStyleVar();
            ImGui::End();
        }


        //Timer
        if (windowState["showTimer"]) {
            ImGui::Begin("Time", 0, ImGuiWindowFlags_NoMove);

            isRobotSleeping = WorkPal3000::isIdling;

            std::string robotHead;
            std::string robotBody;
            std::string robotFeet;

            //Robot blinking
            auto now = std::chrono::steady_clock::now();
            /*
            if (std::chrono::duration_cast<std::chrono::seconds>(now - lastBlinkTime).count() > 1) { // Change the number to adjust the blink speed
                robotOpenEyes = false;
                lastBlinkTime = now;
            }
            */
            if (isRobotIdle) {

                if (robotOpenEyes == false && std::chrono::duration_cast<std::chrono::milliseconds>(now - lastBlinkTime).count() > 200) { //Change to adjust blink length
                    robotOpenEyes = true;
                }
                
                if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastBlinkTime).count() > 2000) { // Change the number to adjust the blink intervals
                    lastBlinkTime = now;
                    robotOpenEyes = false;
                }
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
            if (robotOpenEyes) {
                robotHead = "[^_^]";
            }
            else {
                robotHead = "[-_-]";
            }
   
         
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
            robotFeet = " / \\ ";

            //Assemble robot
            std::vector<std::string> robotParts = { robotHead, robotBody, robotFeet };

            // Invisible child window to enable centering for multiple elements
            ImGui::BeginChild("Centered child window", ImGui::GetContentRegionAvail(), false);

            // Get the window's size
            ImVec2 windowSize = ImGui::GetContentRegionAvail();

         
            // Calculate the position to start drawing text to center it
            float yOffset;
            if (WorkPal3000::isIdling) {
                yOffset = 120.0f;
            }
            else {
                yOffset = 80.0f;
            }

            ImVec2 textPos = ImVec2((windowSize.x) * 0.5f, (windowSize.y) * 0.5f - yOffset); // Subtract offst to move everything up

            // Set the cursor to the calculated position and render the robot
            for (const auto& part : robotParts) {
                ImVec2 partPos = ImVec2((windowSize.x - ImGui::CalcTextSize(part.c_str()).x) * 0.5f, textPos.y);
                ImGui::SetCursorPos(partPos);
                ImGui::Text(part.c_str());
                textPos.y += ImGui::CalcTextSize(part.c_str()).y;
            }

            // Add a small offset to the y-coordinate of the text position
            textPos.y += 30.0f;

            //Idle message
            if (WorkPal3000::isIdling) {

                std::string idleTriggeredText = "Idle triggered";
                std::string idleTimeText = "Idle accumulated: "+WorkPal3000::getIdleTime();
                std::string idleQuestionText = "Have you been working?";
                std::vector<std::string> idleTextSnippets = { idleTriggeredText, idleTimeText, idleQuestionText};
               
                for (const auto& textSnippet : idleTextSnippets) {
                    textPos.x = (windowSize.x - ImGui::CalcTextSize(textSnippet.c_str()).x) * 0.5f;
                    ImGui::SetCursorPos(textPos);
                    ImGui::Text(textSnippet.c_str());
                    textPos.y += ImGui::CalcTextSize(textSnippet.c_str()).y;
                }

                textPos.y += 140;
                textPos.x = (windowSize.x - 600) * 0.5f;
                ImGui::SetCursorPos(textPos);

                if (ImGui::Button("Yes, keep\nidle time", ImVec2(296, 80))) {
                    WorkPal3000::submitIdleResult(true);
                }
                ImGui::SameLine();
                if (ImGui::Button("No, remove\nidle time", ImVec2(296, 80))) {
                    WorkPal3000::submitIdleResult(false);
                }
            }
            else { // Regular Time Message

                std::string timerPreText = "You have been working for:\n";
                textPos.x = (windowSize.x - ImGui::CalcTextSize(timerPreText.c_str()).x) * 0.5f;
                ImGui::SetCursorPos(textPos);
                ImGui::Text(timerPreText.c_str());
                textPos.y += 30.0f;
                textPos.x = (windowSize.x - ImGui::CalcTextSize(WorkPal3000::getElapsedTime().c_str()).x) * 0.5f;
                ImGui::SetCursorPos(textPos);
                ImGui::Text(WorkPal3000::getElapsedTime().c_str());
            }

            
            ImGui::EndChild();
            ImGui::End();
        }

        if (windowState["showSetup"]) {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15 , 15));
            ImGui::Begin("Setup", 0, ImGuiWindowFlags_NoMove);
            ImGui::Text("Please sign in to start using WorkPal3000.\nThis is just to verify your subscription.\nThis is a one time sign in.");

            ImGui::InputTextWithHint("##emailStartup", "Email Address", emailBuffer, IM_ARRAYSIZE(emailBuffer));
            ImGui::InputTextWithHint("##passwordStartup", "Password", passwordBuffer, IM_ARRAYSIZE(passwordBuffer), ImGuiInputTextFlags_Password);

            if (ImGui::Button("Sign In")) {
                WorkPal3000::oneTimeSignIn(emailBuffer, passwordBuffer);
            }

            if (WorkPal3000::invalidLogin) {
                ImGui::Dummy(ImVec2(0, 20));
                ImGui::Text("Invalid Login.\nYour email address or password were incorrect.\nPlease try again or reset your password.");
                if (ImGui::Button("Reset Password")) ShellExecute(0, 0, L"https://workpal3000.com/reset-password-request.php", 0, 0, SW_SHOW);
                
            }

            ImGui::PopStyleVar();
            ImGui::End();
        }

        if (windowState["showSubscription"]) {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
            ImGui::Begin("Subscription", 0);
           
            ImGui::Text("Your subscription is no longer active. This is\nlikely because you cancelled or because your\npayment method has failed.");
            ImGui::Dummy(ImVec2(0, 20));
            ImGui::Text("Use the link below to manage your subscription.");
            ImGui::Dummy(ImVec2(0, 20));
            if (ImGui::Button("Manage Subscription")) ShellExecute(0, 0, L"https://workpal3000.com/login.php", 0, 0, SW_SHOW);
            ImGui::PopStyleVar();
            ImGui::End();
        }




        return windowState;
    }


}