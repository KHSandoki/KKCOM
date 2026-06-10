#pragma once

#include "SerialManager.h"
#include "ConfigManager.h"
#include "ResourceManager.h"
#include "version.h"
#include "textselect.hpp"
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <deque>
#include <queue>
#include <condition_variable>
#include <fstream>
#include <chrono>

class SerialApp {
public:
    SerialApp();
    ~SerialApp();

    bool initialize();
    void run();
    void shutdown();

    // Static method for showing splash screen
    static void showSplashScreen();

private:
    // Core components
    SerialManager serialManager_;
    ConfigManager configManager_;

    // GUI state
    bool showDemo_ = false;
    char inputBuffer_[4096] = "";   // multi-line send buffer
    char filterBuffer_[256] = "";
    int lineEndingMode_ = 3;        // 0=None,1=LF,2=CR,3=CR+LF; appended to command sends
    std::deque<std::string> receivedData_;
    std::string partialLine_;           // incomplete line (no \n yet), displayed live
    std::mutex dataMutex_;
    std::queue<std::string> pendingData_;
    std::mutex pendingMutex_;
    TextSelect textSelect_;
    bool autoScroll_ = true;
    int itemsRemovedFromFront_ = 0;
    float prevScrollY_ = 0.0f;

    // Splitter positions
    float leftPanelSplitterPos_ = 0.7f;  // Position for data display vs input panel
    float connectionPanelHeight_ = 200.0f;  // Height of connection panel

    // Connection state
    std::vector<SerialManager::PortInfo> availablePorts_;
    int selectedPortIndex_ = 0;
    int selectedBaudRate_ = 9; // Index of 115200 in baudRates_ (was 7 = 38400, mislabeled)
    std::vector<int> baudRates_ = {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 921600};
    bool connected_ = false;
    std::string connectionStatus_;  // inline error/status shown in the connection panel

    // Send every functionality
    bool sendEveryEnabled_ = false;
    int sendEveryInterval_ = 1;
    std::thread sendEveryThread_;
    std::atomic<bool> sendEveryRunning_{false};
    std::condition_variable sendEveryCv_;
    std::mutex sendEveryMutex_;

    // Toggle send functionality
    bool toggleSendEnabled_ = false;
    std::thread toggleSendThread_;
    std::atomic<bool> toggleSendRunning_{false};
    std::condition_variable toggleSendCv_;
    std::mutex toggleSendMutex_;
    char toggleCommand0_[256] = "";
    char toggleCommand1_[256] = "";
    int toggleInterval0_ = 1;
    int toggleInterval1_ = 1;

    // Edit state for command button (name + command + color)
    bool showEditWindow_ = false;
    int editTabIndex_ = 0;
    int editGroupIndex_ = 0;
    int editCommandIndex_ = 0;
    char tempEditName_[64] = "";
    char tempEditCmd_[256] = "";
    float tempEditColor_[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    // Group management state
    bool showAddGroupPopup_ = false;
    bool showRenameGroupPopup_ = false;
    bool showGroupColorPopup_ = false;
    int contextTabIndex_ = -1;
    int contextGroupIndex_ = -1;
    char tempGroupName_[64] = "";
    float tempGroupColor_[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    // Pinned commands state
    bool showPinnedEditPopup_ = false;
    int pinnedEditTabIndex_ = 0;
    int pinnedEditCmdIndex_ = -1;
    char tempPinnedName_[64] = "";
    char tempPinnedCmd_[256] = "";
    float tempPinnedColor_[4] = {0.26f, 0.59f, 0.98f, 0.80f};
    int contextPinnedTabIndex_ = -1;
    int contextPinnedIndex_ = -1;

    // Logging state
    bool loggingEnabled_ = true;
    bool timestampEnabled_ = true;
    bool autoFilename_ = true;
    char logFilePath_[512] = "com_log.txt";
    std::ofstream logFile_;
    int logFlushCounter_ = 0;
    std::chrono::steady_clock::time_point lastLogFlush_ = std::chrono::steady_clock::now();
    // Incomplete trailing RX bytes (no newline yet), held so logged lines aren't
    // split mid-line when a logical line spans multiple serial read chunks.
    std::string logPartialLine_;
    // Guards logFile_/logFlushCounter_: logData() runs on the serial RX/TX
    // threads while flushLogIfDue() and start/stopLogging() run on the UI thread.
    std::mutex logMutex_;

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
    // appendEnding=true appends the configured line ending; single keystrokes
    // pass false to send the raw byte with no newline.
    void sendCommand(const std::string& command, bool appendEnding = true);
    const char* lineEndingString() const;
    void refreshPorts();
    void toggleConnection();

    // Threading methods
    void sendEveryLoop();
    void toggleSendLoop();

    // Utility methods
    void clearDataDisplay();
    void drainPendingData();
    void applyFilter();
    void loadConfiguration();
    void saveConfiguration();
    bool splitterV(const char* str_id, float* size1, float* size2, float min_size1, float min_size2);
    void setWindowIcon(GLFWwindow* window);
    std::vector<ExtGroup>& getTabGroups(int tabIndex);
    std::vector<ExtCommand>& getTabPinnedCmds(int tabIndex);

    // Logging methods
    void startLogging();
    void stopLogging();
    void logData(const std::string& data, bool isReceived = true);
    void writeLogLine(const char* prefix, const std::string& line);  // assumes logMutex_ held
    void flushLogIfDue();
    std::string getCurrentTimestamp();
    std::string generateAutoFilename();
    void openFileDialog();

    // Constants
    static const int MAX_DISPLAY_LINES = 1000;
    // While the user has an active text selection, front-trimming is paused to
    // keep TextSelect's line indices stable. This is the safety cap at which we
    // give up and resume trimming even if a selection is held. Kept modest so a
    // selection left active under heavy streaming doesn't bloat the buffer (and
    // make the per-frame TextSelect rebuild expensive).
    static const int MAX_DISPLAY_LINES_HARD_CAP = 10000;
    // Flush buffered log data to disk at least this often, even if fewer than
    // 50 lines have accumulated and logging hasn't stopped.
    static constexpr int LOG_FLUSH_INTERVAL_MS = 1000;
};
