#define CURL_STATICLIB
#define SFML_STATIC
#include "WorkPal3000.h"
#include "curl.h"

#pragma comment (lib, "Normaliz.lib")
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Wldap32.lib")
#pragma comment (lib, "Crypt32.lib")
#pragma comment (lib, "advapi32.lib")

namespace WorkPal3000 {

    //Global variables
    bool isIdling = false;
    std::string version = "1.0.0";
    bool needsOneTimeSetup = true;
    bool invalidLogin = false;
    bool subscriptionActive = false;
    std::string ambientSoundFile = "none";
    std::string currentlyPlayingAmbientSoundFile = "none";
    std::mutex ambientSoundMutex;
    int idleDuration = 1;
    int intervalDuration = 1;
    bool playIntervalSounds = true;

    //Private variables
    std::chrono::duration<double> elapsedTime;
    std::chrono::duration<double> totalElapsedTime(0);
    bool keepRunning = true;
    std::mutex cv_m;
    std::condition_variable cv;
    DWORD idleTime;
    const std::string HOSTS_FILE_PATH = "C:\\Windows\\System32\\drivers\\etc\\hosts";
    std::vector<std::string> blocklist = {};
    std::string currentDate;
    const std::string hostsFileStart = "# WorkPal3000 - Start";
    const std::string hostsFileEnd = "# WorkPal3000 - End";
    bool intervalHappened = false;
    
    void playAmbientSound() {
        sf::Music ambientSound;
        sf::Music intervalSound;

        if (!ambientSound.openFromFile("Assets/"+ambientSoundFile)) std::cerr << "Error loading the file!" << std::endl;
       

        // Play the ambientSound
        if (ambientSoundFile != "none") {
            ambientSound.play();
            ambientSound.setLoop(true);
        }


        // Keep the thread running while the ambientSound is playing
        while (true) {
            if (currentlyPlayingAmbientSoundFile != ambientSoundFile) {
                if (ambientSound.getStatus() == sf::Music::Playing) ambientSound.stop();
                if (ambientSoundFile != "none") {
                    ambientSound.openFromFile("Assets/" + ambientSoundFile);
                    ambientSound.play();
                    ambientSound.setLoop(true);
                }
                currentlyPlayingAmbientSoundFile = ambientSoundFile;
            }
            if (intervalHappened) {
                std::cout << "Interval Triggered" << std::endl;
                intervalSound.openFromFile("Assets/interval-chime.wav");
                intervalSound.play();
                intervalSound.setLoop(false);
                flashWindow();
                ImGui::SetWindowFocus("Time");
                ambientSoundMutex.lock();
                intervalHappened = false;
                ambientSoundMutex.unlock();
            }
            sf::sleep(sf::seconds(0.1f));
        }
    }


   
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

            if (playIntervalSounds) {
                int32_t totalTime = static_cast<int32_t>(std::floor(totalElapsedTime.count()));
                if (totalTime % (intervalDuration * 60) == 0) {
                    ambientSoundMutex.lock();
                    intervalHappened = true;
                    ambientSoundMutex.unlock();
                }
            }
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


    

    // Callback to handle the server's response
    size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }


    void oneTimeSignIn(const char* email, const char* password) {

        
        CURL* curl;
        CURLcode res;
        std::string readBuffer;

        curl = curl_easy_init();
        if (curl) {

            char* encodedEmail = curl_easy_escape(curl, email, strlen(email));
            char* encodedPassword = curl_easy_escape(curl, password, strlen(password));
            std::string postFields = "email=" + std::string(encodedEmail) + "&password=" + std::string(encodedPassword);
            std::string contentLengthHeader = "Content-Length: " + std::to_string(postFields.length());
            curl_free(encodedEmail);
            curl_free(encodedPassword);
           
            curl_easy_setopt(curl, CURLOPT_URL, "https://workpal3000.com/one-time-setup.php");
            curl_easy_setopt(curl, CURLOPT_POST, 1);
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postFields.length());
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "MyApp/1.0");
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

            // Set headers
            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            // headers = curl_slist_append(headers, contentLengthHeader.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            }
            else {

                auto jsonResponse = nlohmann::json::parse(readBuffer.c_str());
                curl_slist_free_all(headers);

                // Check if the response contains a token or an error
                if (jsonResponse.contains("application_key")) {
                    std::string application_key = jsonResponse["application_key"];
                    std::string name = jsonResponse["name"];
                    std::cout << "Received token: " << application_key << "\nUsername: " << name << std::endl;
                    // Handle successful authentication here

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


                    userData["name"] = name;
                    userData["application_key"] = application_key;

                    // Write the data to the file
                    std::ofstream outFile(filename);
                    if (outFile.is_open()) {
                        outFile << userData;
                        validateSubscription(application_key.c_str());
                        needsOneTimeSetup = false;
                        EngineUI::initUI();
                        beginThreads();
                    }
                    else {
                        std::cerr << "Error: Unable to open file for writing.\n";
                    }

                }
                else if (jsonResponse.contains("error")) {
                    std::string error = jsonResponse["error"];
                    std::cerr << "Error: " << error << std::endl;
                    // Handle authentication error here
                    invalidLogin = true;
                }
                
                
                
            }

            curl_easy_cleanup(curl);
        }
        

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

    void validateSubscription(const char* applicationKey) {

        CURL* curl;
        CURLcode res;
        std::string readBuffer;

        curl = curl_easy_init();
        if (curl) {

            char* encodedApplicationKey = curl_easy_escape(curl, applicationKey, strlen(applicationKey));
            std::string postFields = "application_key=" + std::string(applicationKey);
            std::string contentLengthHeader = "Content-Length: " + std::to_string(postFields.length());
            curl_free(encodedApplicationKey);

            curl_easy_setopt(curl, CURLOPT_URL, "https://workpal3000.com/subscription-validation.php");
            curl_easy_setopt(curl, CURLOPT_POST, 1);
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postFields.length());
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "MyApp/1.0");
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); 
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

            // Set headers
            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            // headers = curl_slist_append(headers, contentLengthHeader.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            }
            else {

                auto jsonResponse = nlohmann::json::parse(readBuffer.c_str());
                curl_slist_free_all(headers);

                // Check if the response contains a token or an error
                if (jsonResponse.contains("subscription_active")) {
                    std::string subscriptionActiveResult = jsonResponse["subscription_active"];
                    if (subscriptionActiveResult == "false") {
                        subscriptionActive = false;
                    }
                    else if (subscriptionActiveResult == "true") {
                        subscriptionActive = true;
                    }
                }
                else if (jsonResponse.contains("error")) {
                    std::string error = jsonResponse["error"];
                    std::cerr << "Error: " << error << std::endl;
                    // Handle authentication error here
                    invalidLogin = true;
                }



            }

            curl_easy_cleanup(curl);
        }
    }

    void beginThreads() {
        std::thread ambientSoundThread(playAmbientSound);
        std::thread timer = startTimer();
        std::thread inactivityChecker(checkInactivityThread);
        inactivityChecker.detach();
        ambientSoundThread.detach();
        timer.detach();
    }


    void setup() {

        std::string filename = "userdata.json"; // The file where the data will be stored
        nlohmann::json userData;


        // Try to open the file to read the data
        std::ifstream inFile(filename);
        if (inFile.is_open()) {

            //Get the current date
            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            std::tm tm;
            localtime_s(&tm, &in_time_t);
            std::ostringstream oss;
            oss << std::put_time(&tm, "%Y-%m-%d");
            currentDate = oss.str();

            needsOneTimeSetup = false;
            inFile >> userData;
            std::string name = userData["name"].get<std::string>(); // Extract name as std::string
            std::string key = userData["application_key"];

            if (userData.contains("idleDuration"))idleDuration = userData["idleDuration"].get<int>();
            if (userData.contains("playIntervalSounds")) playIntervalSounds = userData["playIntervalSounds"].get<bool>();
            if (userData.contains("intervalDuration")) intervalDuration = userData["intervalDuration"].get<int>();
            if (userData.contains("ambientSoundFile")) ambientSoundFile = userData["ambientSoundFile"].get<std::string>();

            // Allocate memory for char array including null terminator
            char* keyChar = new char[key.length() + 1];
            strcpy_s(keyChar, key.length() + 1, key.c_str());
            validateSubscription(keyChar);
            delete[] keyChar;

            //Get the blocklist & add it to the hosts file
            if (userData.contains("blocklist")) {
                blocklist = userData["blocklist"];
                addUrlsToHostsFile(blocklist);
            }

            //Get the current seconds
            if (userData["timeData"].contains(currentDate)) {
                std::string durationString = userData["timeData"][currentDate];
                totalElapsedTime = std::chrono::duration<double>(std::stod(durationString));
            }

            //Start the timers
            beginThreads();
        }
        else { //user data file doesnt exist
            needsOneTimeSetup = true;
        }
    }
}



int main()
{
    
   //ShowWindow(GetConsoleWindow(), SW_HIDE);

   WorkPal3000::setup();
   Rendering::start();
   atexit(WorkPal3000::clearHostsFile);
  
   
   return 0;

}