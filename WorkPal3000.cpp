#include "WorkPal3000.h"

namespace WorkPal3000 {

    //Global variables
    bool isIdling = false;
    std::string version = "1.0.0";

    //Private variables
    std::chrono::duration<double> elapsedTime;
    std::chrono::duration<double> totalElapsedTime(0);
    bool keepRunning = true;
    std::mutex cv_m;
    std::condition_variable cv;
    DWORD idleTime;
    int idleDuration = 1;
    const std::string HOSTS_FILE_PATH = "C:\\Windows\\System32\\drivers\\etc\\hosts";
    std::vector<std::string> blocklist = {};
    std::string currentDate;
    const std::string hostsFileStart = "# WorkPal3000 - Start";
    const std::string hostsFileEnd = "# WorkPal3000 - End";
    
   
    void timerThread(const std::chrono::high_resolution_clock::time_point& startTime) {
        std::chrono::high_resolution_clock::time_point lastTime = startTime;

        while (true) {
            // This will wait for 1 second or until notified to wake up
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - lastTime);
            totalElapsedTime += elapsedTime;

            lastTime = currentTime; // Update lastTime for the next iteration

            saveTime();

        }
    }

    double getSecondsOnDate(std::string date) {
        nlohmann::json userData;
        std::ifstream inFile("userdata.json");
        if (inFile.is_open()) {
            inFile >> userData;
            if (userData["timeData"].contains(date)) {
                std::string stringSeconds = userData["timeData"][date];
                double seconds = std::stod(stringSeconds);
                return seconds;
            }
            else {
                return 0;
            }
        }
        else {
            std::cout << "Unable to open the file." << std::endl;
            return 0;
        }
    }

    int currentWeekOfYear() {
        std::time_t t = std::time(0);   // get time now
        std::tm now;
        localtime_s(&now, &t);

        int dayOfYear = now.tm_yday + 1;
        int wday = now.tm_wday;
        if (wday == 0) wday = 7;  // make Sunday day 7
        int startDayOfYear = (dayOfYear - wday + 10) / 7;  // week 1 of the year is the one that includes January 4

        return startDayOfYear;
    }

    std::vector<std::string> getDatesInWeek(int weekNumber) {
        // Define the vector that will hold the dates
        std::vector<std::string> dates;

        // Use the chrono library to get the current year
        auto now = std::chrono::system_clock::now();
        auto currentTime = std::chrono::system_clock::to_time_t(now);
        std::tm dateInfo;
        localtime_s(&dateInfo, &currentTime);
        int currentYear = dateInfo.tm_year + 1900;

        // Determine the date of the first Monday of the year
        std::tm firstDay = { 0, 0, 0, 1, 0, currentYear - 1900 };  // January 1st of the current year
        std::time_t firstDayTime = std::mktime(&firstDay);
        std::tm firstMonday;
        localtime_s(&firstMonday, &firstDayTime);
        while (firstMonday.tm_wday != 1) {  // Monday is 1 in tm_wday
            firstDayTime += 24 * 60 * 60;  // add a day
            localtime_s(&firstMonday, &firstDayTime);
        }

        // Add the number of seconds in a week times the week number
        std::time_t weekDayTime = firstDayTime + (weekNumber - 1) * 7 * 24 * 60 * 60;

        for (int i = 0; i < 7; i++) {
            std::tm weekDayInfo;
            localtime_s(&weekDayInfo, &weekDayTime);
            char buffer[11];
            std::strftime(buffer, 11, "%Y-%m-%d", &weekDayInfo);

            // Add the date to the vector
            dates.push_back(buffer);

            // Move to the next day
            weekDayTime += 24 * 60 * 60;
        }

        return dates;
    }



    void saveTime() {

        int totalSeconds = static_cast<int>(totalElapsedTime.count());

        nlohmann::json userData;
        std::ifstream inFile("userdata.json");
        if (inFile.is_open()) {
            inFile >> userData;
            if (userData["timeData"][currentDate] == NULL) {
                userData["timeData"][currentDate] = 0;
            }
            else {
                userData["timeData"][currentDate] = std::to_string(totalSeconds);
            }
        }

        //save changes
        std::ofstream outFile("userdata.json");
        if (outFile.is_open()) {
            outFile << userData;
        }

    }

    std::thread startTimer() {
        std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();
        std::thread timer(timerThread, startTime);
        return timer;
    }

    void printElapsedTime() {
        int totalSeconds = static_cast<int>(totalElapsedTime.count());
        int hours = totalSeconds / 3600;
        int minutes = (totalSeconds % 3600) / 60;
        int seconds = totalSeconds % 60;
        std::cout << "You have been working for: " << hours << " hours, " << minutes << " minutes, " << seconds << " seconds\n";
    }

    DWORD GetWindowsIdleMilliseconds() {
        LASTINPUTINFO lii;
        lii.cbSize = sizeof(LASTINPUTINFO);
        GetLastInputInfo(&lii);
        return GetTickCount() - lii.dwTime;
    }

    void flashWindow() {

        Rendering::flashWindowsEx();

        glfwHideWindow(Rendering::window);
        glfwPollEvents(); // Process events
        glfwWaitEventsTimeout(0.2); // Wait for a bit
        glfwShowWindow(Rendering::window);
    }


    void checkInactivityThread() {
        std::unique_lock<std::mutex> lock(cv_m);
        while (keepRunning) {
            // This will wait for 1 second or until notified to wake up
            if (cv.wait_for(lock, std::chrono::seconds(1), [] { return !keepRunning; })) {
                break;
            }

            if (isIdling) {
                idleTime += 1000;
            }
            else {
                idleTime = GetWindowsIdleMilliseconds();
            }

            if (idleTime >= (idleDuration * 60 * 1000) && !isIdling) {
            
                isIdling = true;
                flashWindow();
                ImGui::SetWindowFocus("Time");
            }
        }
    }

    void submitBlocklist(std::vector<std::string> newBlocklist) {

 
        totalElapsedTime = std::chrono::duration<double>(0);

        //Remove old blocklist from hosts file
        clearHostsFile();

        //Add the new blocklist to userdata
        for (size_t i = 0; i < newBlocklist.size(); i++) addUrlToBlockList(newBlocklist[i], i);

        //Add to the hosts file
        addUrlsToHostsFile(newBlocklist);


    }

    void addUrlToBlockList(std::string url, int i) {

        if (url == "") return;

        blocklist[i] = url;
           
        nlohmann::json userData;
        std::ifstream inFile("userdata.json");
        if (inFile.is_open()) {
            inFile >> userData;
            userData["blocklist"] = blocklist;
            std::ofstream outFile("userdata.json");
            if (outFile.is_open()) {
                outFile << userData;
                std::cerr << "Updated block list succesfully!\n";
            }
        }
       

    }

    void getHoursInYear(double yearData[12]) {

        // Assuming the array is already initialized to zeros
        for (int i = 0; i < 12; ++i)  yearData[i] = 0;
        
        nlohmann::json userData;
        std::ifstream inFile("userdata.json");
        if (inFile.is_open()) {
            inFile >> userData;
            if (userData.contains("timeData")) {
                for (auto& item : userData["timeData"].items())
                {
                    std::string dateString = item.key();
                    std::string monthString = dateString.substr(5, 2); // Extract the month part
                    int month = std::stoi(monthString); // Convert to integer to remove leading zero
                    std::string secondsString = item.value();
                    double seconds = std::stod(secondsString);
                    double hours = seconds / 3600;
                    yearData[month - 1] += hours;
                }
            }
        }
    }



    void addUrlsToHostsFile(const std::vector<std::string>& urls) {
        bool inMyAppSection = false;
        bool sectionExists = false;
        bool urlsWritten = false;
        std::string newHostsContents;
        std::ifstream hostsFile(HOSTS_FILE_PATH);
        std::string line;

        while (getline(hostsFile, line)) {
            if (line.find(hostsFileEnd) != std::string::npos) {
                inMyAppSection = false;
                urlsWritten = false; // Reset urlsWritten when leaving a section
            }

            // Check if we're in the section
            if (inMyAppSection && !urlsWritten) {
                for (const auto& url : urls) {
                    if (!isUrlInHostsFile(url)) {
                        newHostsContents += "127.0.0.1 " + url + "\n";
                    }
                }
                urlsWritten = true; // Set urlsWritten to true after writing URLs
            }

            // Always add the line to new contents
            newHostsContents += line + "\n";

            if (line.find(hostsFileStart) != std::string::npos) {
                sectionExists = true;
                inMyAppSection = true;
            }
        }

        if (!sectionExists) {
            // If the section doesn't exist, add it at the end of the file.
            newHostsContents += hostsFileStart + "\n";
            for (const auto& url : urls) {
                if (!isUrlInHostsFile(url)) {
                    newHostsContents += "127.0.0.1 " + url + "\n";
                }
            }
            newHostsContents += hostsFileEnd + "\n";
        }

        hostsFile.close();
        std::ofstream hostsFileOut(HOSTS_FILE_PATH);
        hostsFileOut << newHostsContents;

        // Flush DNS cache
        system("ipconfig /flushdns");
    }
    void removeUrlsFromHostsFile(const std::vector<std::string>& urls) {
        bool inMyAppSection = false;
        std::string newHostsContents;
        std::ifstream hostsFile(HOSTS_FILE_PATH);
        std::string line;

        while (getline(hostsFile, line)) {
            bool shouldRemove = false;

            if (line.find(hostsFileEnd) != std::string::npos) {
                inMyAppSection = false;
            }

            if (inMyAppSection) {
                for (const auto& url : urls) {
                    if (line.find(url) != std::string::npos) {
                        shouldRemove = true;
                        break;
                    }
                }
            }

            if (!shouldRemove) {
                newHostsContents += line + "\n";
            }

            if (line.find(hostsFileStart) != std::string::npos) {
                inMyAppSection = true;
            }
        }

        hostsFile.close();
        std::ofstream hostsFileOut(HOSTS_FILE_PATH);
        hostsFileOut << newHostsContents;


    }



    void removeUrlFromBlockList(std::string url) {

        bool urlExists = false;

        for (size_t i = 0; i < blocklist.size(); i++)
        {

            if (blocklist[i] == url) {
                //Remove it from the save file
                blocklist[i] = "";

                nlohmann::json userData;
                std::ifstream inFile("userdata.json");
                if (inFile.is_open()) {
                    inFile >> userData;

                    if (blocklist.size() == 1 && blocklist[0] == "") {
                        userData.erase("blocklist");
                    }
                    else {
                        userData["blocklist"] = blocklist;
                    }

                    std::ofstream outFile("userdata.json");
                    if (outFile.is_open()) {
                        outFile << userData;
                        std::cerr << "Removed url succesfully!\n";
                        return;
                    }
                }
            }

        }


    }

    bool isUrlInHostsFile(const std::string& url) {
        std::ifstream hostsFile("C:\\Windows\\System32\\drivers\\etc\\hosts"); // on Windows use "C:\\Windows\\System32\\drivers\\etc\\hosts"

        std::string line;
        while (getline(hostsFile, line)) {
            if (line.find(url) != std::string::npos) {
                return true;
            }
        }

        return false;
    }



    void submitIdleResult(bool wasWorking) {

        if (wasWorking) {
            isIdling = false;
            idleTime = 0;
        }
        else {
            int totalIdleSeconds;
            {
                totalIdleSeconds = idleTime / 1000; // Convert milliseconds to seconds
                idleTime = 0;
                isIdling = false;
            }
            {
                std::chrono::seconds idleDuration(totalIdleSeconds);
                totalElapsedTime -= idleDuration;
            }
        }
    }

    std::string getElapsedTime() {
        int totalSeconds = static_cast<int>(totalElapsedTime.count());
        int hours = totalSeconds / 3600;
        int minutes = (totalSeconds % 3600) / 60;
        int seconds = totalSeconds % 60;
        std::string time;

        if (hours == 1) {
            time += "1 Hour ";
        }
        else if (hours > 1) {
            time += std::to_string(hours) + " Hours ";
        }

        if (minutes == 1) {
            time += "1 Minute";
        }
        else if (minutes > 1) {
            time += std::to_string(minutes) + " Minutes";
        }

        if (hours == 0 && minutes == 0 && seconds == 1) {
            time += "1 Second";
        }
        else if (hours == 0 && minutes == 0) {
            time += std::to_string(seconds) + " Seconds";
        }

        return time;
    }

    std::string getIdleTime() {

        int totalIdleSeconds;
        totalIdleSeconds = idleTime / 1000; // Convert milliseconds to seconds
        int hours = totalIdleSeconds / 3600;
        int minutes = (totalIdleSeconds % 3600) / 60;
        int seconds = totalIdleSeconds % 60;
        std::string time;

        if (hours == 1) {
            time += "1 Hour ";
        }
        else if (hours > 1) {
            time += std::to_string(hours) + " Hours ";
        }

        if (minutes == 1) {
            time += "1 Minute";
        }
        else if (minutes > 1) {
            time += std::to_string(minutes) + " Minutes";
        }

        if (hours == 0 && minutes == 0 && seconds == 1) {
            time += "1 Second";
        }
        else if (hours == 0 && minutes == 0) {
            time += std::to_string(seconds) + " Seconds";
        }

        return time;
    }


    void setup() {
      
        std::string filename = "userdata.json"; // The file where the data will be stored
        nlohmann::json userData;

        //Get the current date
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm tm;
        localtime_s(&tm, &in_time_t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d");
        currentDate = oss.str();

        // Try to open the file to read the data
        std::ifstream inFile(filename);
        if (inFile.is_open()) {
            inFile >> userData;
            std::string name = userData["name"].get<std::string>(); // Extract name as std::string

            //Get the blocklist & add it to the hosts file
            if (userData.contains("blocklist")) {
                blocklist = userData["blocklist"];
                addUrlsToHostsFile(blocklist);
            }

           

            //Get the current seconds
            if (userData["timeData"].contains(currentDate)){
                std::string durationString = userData["timeData"][currentDate];
                totalElapsedTime = std::chrono::duration<double>( std::stod(durationString) );
            }

        }
        else {
            userData["name"] = "Jake";

            // Write the data to the file
            std::ofstream outFile(filename);
            if (outFile.is_open()) {
                outFile << userData;
            }
            else {
                std::cerr << "Error: Unable to open file for writing.\n";
            }
        }

    }

    void start() {

        setup();

        std::thread timer = startTimer();
        std::thread inactivityChecker(checkInactivityThread);

        inactivityChecker.detach();
        timer.detach();  


        
    }

    void clearHostsFile()
    {
        for (int i = 0; i < WorkPal3000::blocklist.size(); ++i) removeUrlsFromHostsFile(blocklist);
        bool inMyAppSection = false;
        std::string newHostsContents;
        std::ifstream hostsFile(HOSTS_FILE_PATH);
        std::string line;

        while (getline(hostsFile, line)) {
            if (line.find(hostsFileStart) != std::string::npos) {
                inMyAppSection = true;
                continue;
            }

            if (line.find(hostsFileEnd) != std::string::npos) {
                inMyAppSection = false;
                continue;
            }

            if (!inMyAppSection) {
                newHostsContents += line + "\n";
            }
        }

        hostsFile.close();
        std::ofstream hostsFileOut(HOSTS_FILE_PATH);
        hostsFileOut << newHostsContents;

        system("ipconfig /flushdns");
    }
}

int main()
{
   ShowWindow(GetConsoleWindow(), SW_HIDE);
   WorkPal3000::start();
   Rendering::start();
   atexit(WorkPal3000::clearHostsFile);
   return 0;

}