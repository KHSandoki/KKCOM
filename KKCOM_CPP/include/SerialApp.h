#pragma once

#include "SerialManager.h"
#include "ConfigManager.h"
#include "version.h"
#include <imgui.h>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <deque>
#include <fstream>

class SerialApp {
public:
    SerialApp();
    ~SerialApp();
    
    bool initialize();
    void run();
    void shutdown();
    
private:
    // Core components
    SerialManager serialManager_;
    ConfigManager configManager_;
    
    // GUI state
    bool showDemo_ = false;
    char inputBuffer_[256] = "";
    char filterBuffer_[256] = "";
    std::deque<std::string> receivedData_;
    std::mutex dataMutex_;
    
    // Splitter positions
    float leftPanelSplitterPos_ = 0.7f;  // Position for data display vs input panel
    float connectionPanelHeight_ = 200.0f;  // Height of connection panel
    
    // Connection state
    std::vector<SerialManager::PortInfo> availablePorts_;
    int selectedPortIndex_ = 0;
    int selectedBaudRate_ = 7; // Index for 115200
    std::vector<int> baudRates_ = {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 921600};
    bool connected_ = false;
    
    // Send every functionality
    bool sendEveryEnabled_ = false;
    int sendEveryInterval_ = 1;
    std::thread sendEveryThread_;
    std::atomic<bool> sendEveryRunning_{false};
    
    // Toggle send functionality  
    bool toggleSendEnabled_ = false;
    std::thread toggleSendThread_;
    std::atomic<bool> toggleSendRunning_{false};
    char toggleCommand0_[256] = "";
    char toggleCommand1_[256] = "";
    int toggleInterval0_ = 1;
    int toggleInterval1_ = 1;
    
    // EXT commands
    char extCommands_[3][100][256]; // 3 tabs, 100 commands each
    char extNames_[3][100][64];     // Command names/labels
    
    // Edit state
    bool showEditWindow_ = false;
    int editTabIndex_ = 0;
    int editCommandIndex_ = 0;
    char tempEditName_[64] = "";
    
    // Logging state
    bool loggingEnabled_ = false;
    bool timestampEnabled_ = true;
    bool autoFilename_ = false;
    char logFilePath_[512] = "com_log.txt";
    std::ofstream logFile_;
    
    // GUI methods
    void renderMainWindow();
    void renderConnectionPanel();
    void renderInputPanel();
    void renderDataDisplay();
    void renderExtTabs();
    void renderExtTab(int tabIndex, const char* tabName);
    void renderTogglePanel();
    void renderEditWindow();
    
    // Serial communication
    void onDataReceived(const std::string& data);
    void sendCommand(const std::string& command);
    void refreshPorts();
    void toggleConnection();
    
    // Threading methods
    void sendEveryLoop();
    void toggleSendLoop();
    
    // Utility methods
    void clearDataDisplay();
    void applyFilter();
    void loadConfiguration();
    void saveConfiguration();
    bool splitterV(const char* str_id, float* size1, float* size2, float min_size1, float min_size2);
    
    // Logging methods
    void startLogging();
    void stopLogging();
    void logData(const std::string& data, bool isReceived = true);
    std::string getCurrentTimestamp();
    std::string generateAutoFilename();
    void openFileDialog();
    
    // Constants
    static const int MAX_DISPLAY_LINES = 1000;
    static const int EXT_COMMAND_COUNT = 100;
};