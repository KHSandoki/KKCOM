#include "SerialApp.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <sstream>
#include <iomanip>
#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

SerialApp::SerialApp() : configManager_("config.json") {
    // Initialize buffers
    memset(inputBuffer_, 0, sizeof(inputBuffer_));
    memset(filterBuffer_, 0, sizeof(filterBuffer_));
    memset(extCommands_, 0, sizeof(extCommands_));
    memset(extNames_, 0, sizeof(extNames_));
    memset(toggleCommand0_, 0, sizeof(toggleCommand0_));
    memset(toggleCommand1_, 0, sizeof(toggleCommand1_));
    
    // Initialize EXT command names with default values
    for (int tab = 0; tab < 3; ++tab) {
        for (int i = 0; i < EXT_COMMAND_COUNT; ++i) {
            snprintf(extNames_[tab][i], sizeof(extNames_[tab][i]), "%d", i);
        }
    }
}

SerialApp::~SerialApp() {
    stopLogging();
    shutdown();
}

bool SerialApp::initialize() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    // Create window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(1200, 800, "KKCOM - C++ Edition", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    
    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Setup style
    ImGui::StyleColorsDark();
    
    // Load Consolas font at 24px
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 24.0f);
    
    // Scale UI elements to match font size
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(1.5f);
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    // Load configuration
    loadConfiguration();
    
    // Setup serial data callback
    serialManager_.setDataCallback([this](const std::string& data) {
        onDataReceived(data);
    });
    
    // Refresh available ports
    refreshPorts();
    
    // Main loop
    // Frame rate limiting
    auto lastFrameTime = std::chrono::high_resolution_clock::now();
    const auto targetFrameTime = std::chrono::milliseconds(16); // 60 FPS
    
    while (!glfwWindowShouldClose(window)) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto deltaTime = currentTime - lastFrameTime;
        
        if (deltaTime >= targetFrameTime) {
            glfwPollEvents();
            
            // Reduce rendering frequency when window is not focused
            bool windowFocused = glfwGetWindowAttrib(window, GLFW_FOCUSED);
            if (!windowFocused && deltaTime < std::chrono::milliseconds(100)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }
        
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // Render our GUI
        renderMainWindow();
        
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
            lastFrameTime = currentTime;
        } else {
            // Sleep to prevent busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return true;
}

void SerialApp::run() {
    initialize();
}

void SerialApp::shutdown() {
    // Stop all threads
    sendEveryRunning_ = false;
    toggleSendRunning_ = false;
    
    if (sendEveryThread_.joinable()) {
        sendEveryThread_.join();
    }
    
    if (toggleSendThread_.joinable()) {
        toggleSendThread_.join();
    }
    
    // Disconnect serial
    serialManager_.disconnect();
    
    // Save configuration
    saveConfiguration();
}

void SerialApp::renderMainWindow() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | 
                                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
                                   ImGuiWindowFlags_MenuBar;
    
    std::string windowTitle = std::string(KKCOM_PRODUCT_NAME) + " v" + KKCOM_VERSION_STRING;
    ImGui::Begin(windowTitle.c_str(), nullptr, window_flags);
    
    // Menu bar
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save Config")) {
                saveConfiguration();
            }
            if (ImGui::MenuItem("Load Config")) {
                loadConfiguration();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Show Demo", nullptr, &showDemo_);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Log")) {
            ImGui::Checkbox("Enable Logging", &loggingEnabled_);
            if (loggingEnabled_ != logFile_.is_open()) {
                if (loggingEnabled_) {
                    startLogging();
                } else {
                    stopLogging();
                }
            }
            
            ImGui::Separator();
            ImGui::Checkbox("Include Timestamp", &timestampEnabled_);
            
            ImGui::Separator();
            bool prevAutoFilename = autoFilename_;
            ImGui::Checkbox("Auto Generate Filename", &autoFilename_);
            if (autoFilename_ && !prevAutoFilename) {
                // Only generate new filename when checkbox is first checked
                std::string autoName = generateAutoFilename();
                strcpy(logFilePath_, autoName.c_str());
            }
            
            ImGui::Separator();
            ImGui::Text("Log File Path:");
            if (!autoFilename_) {
                ImGui::InputText("##LogPath", logFilePath_, sizeof(logFilePath_));
                ImGui::SameLine();
                if (ImGui::Button("Browse...")) {
                    openFileDialog();
                }
            } else {
                ImGui::TextDisabled("%s", logFilePath_);
                if (ImGui::Button("Generate New")) {
                    std::string autoName = generateAutoFilename();
                    strcpy(logFilePath_, autoName.c_str());
                }
            }
            
            ImGui::Separator();
            if (ImGui::Button("Clear Log File")) {
                if (logFile_.is_open()) {
                    logFile_.close();
                    logFile_.open(logFilePath_, std::ios::trunc);
                }
            }
            
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    
    // Main layout
    ImGui::Columns(2, "MainColumns", true);
    
    // Left column - Data display and input with splitter
    float availableHeight = ImGui::GetContentRegionAvail().y;
    float dataDisplayHeight = availableHeight * leftPanelSplitterPos_;
    float inputPanelHeight = availableHeight * (1.0f - leftPanelSplitterPos_);
    
    ImGui::BeginChild("DataDisplayArea", ImVec2(0, dataDisplayHeight), true);
    renderDataDisplay();
    ImGui::EndChild();
    
    // Vertical splitter
    splitterV("LeftPanelSplitter", &dataDisplayHeight, &inputPanelHeight, 100.0f, 100.0f);
    leftPanelSplitterPos_ = dataDisplayHeight / availableHeight;
    
    ImGui::BeginChild("InputPanelArea", ImVec2(0, inputPanelHeight), true);
    renderInputPanel();
    ImGui::EndChild();
    
    ImGui::NextColumn();
    
    // Right column - Connection and EXT tabs with resizable connection panel
    float rightAvailableHeight = ImGui::GetContentRegionAvail().y;
    float extTabsHeight = rightAvailableHeight - connectionPanelHeight_;
    
    ImGui::BeginChild("ConnectionPanelArea", ImVec2(0, connectionPanelHeight_), true);
    renderConnectionPanel();
    ImGui::EndChild();
    
    // Vertical splitter for connection panel
    splitterV("ConnectionPanelSplitter", &connectionPanelHeight_, &extTabsHeight, 150.0f, 100.0f);
    
    ImGui::BeginChild("ExtTabsArea", ImVec2(0, extTabsHeight), true);
    renderExtTabs();
    ImGui::EndChild();
    
    ImGui::Columns(1);
    
    ImGui::End();
    
    // Render edit window
    renderEditWindow();
    
    // Show demo window if requested
    if (showDemo_) {
        ImGui::ShowDemoWindow(&showDemo_);
    }
}

void SerialApp::renderConnectionPanel() {
    ImGui::Text("Connection Settings");
    ImGui::Separator();
    
    // Connection button
    ImVec4 buttonColor = connected_ ? ImVec4(1.0f, 0.2f, 0.2f, 1.0f) : ImVec4(0.1f, 0.7f, 0.1f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    if (ImGui::Button(connected_ ? "Disconnect" : "Connect", ImVec2(-1, 40))) {
        toggleConnection();
    }
    ImGui::PopStyleColor();
    
    // Port selection
    ImGui::Text("COM Port:");
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        refreshPorts();
    }
    
    if (!availablePorts_.empty()) {
        std::vector<const char*> portCStrings;
        for (const auto& port : availablePorts_) {
            portCStrings.push_back(port.c_str());
        }
        ImGui::Combo("##Port", &selectedPortIndex_, portCStrings.data(), static_cast<int>(portCStrings.size()));
    }
    
    // Baud rate selection
    ImGui::Text("Baud Rate:");
    const char* baudRateStrings[] = {"300", "600", "1200", "2400", "4800", "9600", "19200", "38400", "57600", "115200", "921600"};
    ImGui::Combo("##BaudRate", &selectedBaudRate_, baudRateStrings, IM_ARRAYSIZE(baudRateStrings));
}

void SerialApp::renderInputPanel() {
    ImGui::Text("Send Command");
    ImGui::Separator();
    
    // Input field
    ImGui::PushItemWidth(-150);
    bool enterPressed = ImGui::InputText("##Input", inputBuffer_, sizeof(inputBuffer_), ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::PopItemWidth();
    
    ImGui::SameLine();
    if (ImGui::Button("Send") || enterPressed) {
        if (strlen(inputBuffer_) > 0) {
            sendCommand(std::string(inputBuffer_));
        }
    }
    
    // Send every functionality
    ImGui::Checkbox("Send Every", &sendEveryEnabled_);
    if (sendEveryEnabled_ != sendEveryRunning_) {
        if (sendEveryEnabled_) {
            sendEveryRunning_ = true;
            sendEveryThread_ = std::thread(&SerialApp::sendEveryLoop, this);
        } else {
            sendEveryRunning_ = false;
            if (sendEveryThread_.joinable()) {
                sendEveryThread_.join();
            }
        }
    }
    
    if (sendEveryEnabled_) {
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
        ImGui::InputInt("sec", &sendEveryInterval_);
        if (sendEveryInterval_ < 1) sendEveryInterval_ = 1;
        ImGui::PopItemWidth();
    }
    
    // Filter and clear buttons
    if (ImGui::Button("Clear Display")) {
        clearDataDisplay();
    }
    ImGui::SameLine();
    if (ImGui::Button("Set Filter")) {
        // Open filter dialog
        ImGui::OpenPopup("Filter Dialog");
    }
    
    // Filter dialog
    if (ImGui::BeginPopupModal("Filter Dialog", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter filter string:");
        ImGui::InputText("##Filter", filterBuffer_, sizeof(filterBuffer_));
        
        if (ImGui::Button("Apply")) {
            applyFilter();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear Filter")) {
            memset(filterBuffer_, 0, sizeof(filterBuffer_));
            configManager_.getConfig().filterString = "";
            configManager_.getConfig().filterActive = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void SerialApp::renderDataDisplay() {
    ImGui::Text("Received Data (Click to focus for single-key sending)");
    ImGui::Separator();

    ImGui::BeginChild("DataScrolling", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

    // If this window is focused, capture character input
    if (ImGui::IsWindowFocused()) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.InputQueueCharacters.Size > 0) {
            for (int i = 0; i < io.InputQueueCharacters.Size; i++) {
                char c = io.InputQueueCharacters[i];
                if (c >= 32 || c == '\n' || c == '\r' || c == '\t') { // Filter out control characters
                    sendCommand(std::string(1, c));
                }
            }
            // Clear the input queue to prevent processing these characters elsewhere
            io.InputQueueCharacters.resize(0);
        }
    }

    std::lock_guard<std::mutex> lock(dataMutex_);

    // Use ImGui clipper for efficient rendering of large lists
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(receivedData_.size()));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
            ImGui::TextUnformatted(receivedData_[i].c_str());
        }
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
}

void SerialApp::renderExtTabs() {
    if (ImGui::BeginTabBar("ExtTabBar")) {
        if (ImGui::BeginTabItem("EXT 1")) {
            renderExtTab(0, "EXT 1");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("EXT 2")) {
            renderExtTab(1, "EXT 2");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("EXT 3")) {
            renderExtTab(2, "EXT 3");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Toggle")) {
            renderTogglePanel();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    
    if (ImGui::Button("Save Config")) {
        saveConfiguration();
    }
}

void SerialApp::renderExtTab(int tabIndex, const char* tabName) {
    ImGui::BeginChild("ExtCommands", ImVec2(0, 0), false);
    
    ImGui::Columns(2, "ExtColumns", true);
    ImGui::Text("Command");
    ImGui::NextColumn();
    ImGui::Text("Send (Double-click to edit)");
    ImGui::NextColumn();
    ImGui::Separator();
    
    for (int i = 0; i < EXT_COMMAND_COUNT; ++i) {
        ImGui::PushID(i);
        
        // Command input
        ImGui::PushItemWidth(-1);
        ImGui::InputText("##Command", extCommands_[tabIndex][i], sizeof(extCommands_[tabIndex][i]));
        ImGui::PopItemWidth();
        ImGui::NextColumn();
        
        // Send button with double-click edit
        if (ImGui::Button(extNames_[tabIndex][i])) {
            // Single-click: send command
            if (strlen(extCommands_[tabIndex][i]) > 0) {
                sendCommand(std::string(extCommands_[tabIndex][i]));
            }
        }
        
        // Check for double-click on the button
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            // Double-click: open edit window
            showEditWindow_ = true;
            editTabIndex_ = tabIndex;
            editCommandIndex_ = i;
            strcpy(tempEditName_, extNames_[tabIndex][i]);
        }
        ImGui::NextColumn();
        ImGui::PopID();
    }
    
    ImGui::Columns(1);
    ImGui::EndChild();
}

void SerialApp::renderTogglePanel() {
    ImGui::Text("Toggle Send");
    ImGui::Separator();
    
    ImGui::Columns(2, "ToggleColumns", true);
    
    // Toggle commands
    ImGui::Text("Command 0:");
    ImGui::InputText("##ToggleCmd0", toggleCommand0_, sizeof(toggleCommand0_));
    ImGui::Text("Interval 0 (sec):");
    ImGui::InputInt("##ToggleInt0", &toggleInterval0_);
    if (toggleInterval0_ < 1) toggleInterval0_ = 1;
    
    ImGui::NextColumn();
    
    ImGui::Text("Command 1:");
    ImGui::InputText("##ToggleCmd1", toggleCommand1_, sizeof(toggleCommand1_));
    ImGui::Text("Interval 1 (sec):");
    ImGui::InputInt("##ToggleInt1", &toggleInterval1_);
    if (toggleInterval1_ < 1) toggleInterval1_ = 1;
    
    ImGui::Columns(1);
    
    // Toggle control
    ImGui::Checkbox("Toggle Send Active", &toggleSendEnabled_);
    if (toggleSendEnabled_ != toggleSendRunning_) {
        if (toggleSendEnabled_) {
            toggleSendRunning_ = true;
            toggleSendThread_ = std::thread(&SerialApp::toggleSendLoop, this);
        } else {
            toggleSendRunning_ = false;
            if (toggleSendThread_.joinable()) {
                toggleSendThread_.join();
            }
        }
    }
}

void SerialApp::onDataReceived(const std::string& data) {
    std::lock_guard<std::mutex> lock(dataMutex_);
    
    // Log received data
    logData(data, true);
    
    // Apply filter if active
    const auto& config = configManager_.getConfig();
    if (config.filterActive && !config.filterString.empty()) {
        if (data.find(config.filterString) == std::string::npos) {
            return; // Skip this data
        }
    }
    
    receivedData_.push_back(data);
    
    // Limit the number of stored lines
    if (receivedData_.size() > MAX_DISPLAY_LINES) {
        receivedData_.pop_front();
    }
}

void SerialApp::renderEditWindow() {
    if (showEditWindow_) {
        ImGui::OpenPopup("Edit Button Name");
        showEditWindow_ = false; // Only open once
    }
    
    if (ImGui::BeginPopupModal("Edit Button Name", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter button name:");
        ImGui::InputText("##Name", tempEditName_, sizeof(tempEditName_));
        
        if (ImGui::Button("OK")) {
            strcpy(extNames_[editTabIndex_][editCommandIndex_], tempEditName_);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void SerialApp::sendCommand(const std::string& command) {
    if (serialManager_.isConnected()) {
        // Log sent data
        logData(command, false);
        serialManager_.sendData(command);
    }
}

void SerialApp::refreshPorts() {
    availablePorts_ = serialManager_.getAvailablePorts();
    if (selectedPortIndex_ >= static_cast<int>(availablePorts_.size())) {
        selectedPortIndex_ = 0;
    }
}

void SerialApp::toggleConnection() {
    if (connected_) {
        serialManager_.disconnect();
        connected_ = false;
    } else {
        if (!availablePorts_.empty() && selectedPortIndex_ < static_cast<int>(availablePorts_.size())) {
            const std::string& selectedPort = availablePorts_[selectedPortIndex_];
            int baudRate = baudRates_[selectedBaudRate_];
            
            if (serialManager_.connect(selectedPort, baudRate)) {
                connected_ = true;
                configManager_.getConfig().lastPort = selectedPort;
                configManager_.getConfig().lastBaudRate = baudRate;
            }
        }
    }
}

void SerialApp::sendEveryLoop() {
    while (sendEveryRunning_) {
        if (strlen(inputBuffer_) > 0) {
            sendCommand(std::string(inputBuffer_));
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(sendEveryInterval_));
    }
}

void SerialApp::toggleSendLoop() {
    bool sendFirst = true;
    
    while (toggleSendRunning_) {
        if (sendFirst) {
            if (strlen(toggleCommand0_) > 0) {
                sendCommand(std::string(toggleCommand0_));
            }
            std::this_thread::sleep_for(std::chrono::seconds(toggleInterval0_));
        } else {
            if (strlen(toggleCommand1_) > 0) {
                sendCommand(std::string(toggleCommand1_));
            }
            std::this_thread::sleep_for(std::chrono::seconds(toggleInterval1_));
        }
        sendFirst = !sendFirst;
    }
}

void SerialApp::clearDataDisplay() {
    std::lock_guard<std::mutex> lock(dataMutex_);
    receivedData_.clear();
}

void SerialApp::applyFilter() {
    auto& config = configManager_.getConfig();
    config.filterString = std::string(filterBuffer_);
    config.filterActive = !config.filterString.empty();
}

void SerialApp::loadConfiguration() {
    if (configManager_.loadConfig()) {
        const auto& config = configManager_.getConfig();
        
        // Load EXT commands
        for (int tab = 0; tab < 3; ++tab) {
            const auto* commands = (tab == 0) ? &config.ext1Commands : 
                                  (tab == 1) ? &config.ext2Commands : &config.ext3Commands;
            
            for (int i = 0; i < EXT_COMMAND_COUNT && i < static_cast<int>(commands->size()); ++i) {
                strcpy(extCommands_[tab][i], (*commands)[i].command.c_str());
                strcpy(extNames_[tab][i], (*commands)[i].name.c_str());
            }
        }
        
        // Load toggle commands
        strcpy(toggleCommand0_, config.toggleCommand0.c_str());
        strcpy(toggleCommand1_, config.toggleCommand1.c_str());
        toggleInterval0_ = config.toggleInterval0;
        toggleInterval1_ = config.toggleInterval1;
        
        // Load filter
        strcpy(filterBuffer_, config.filterString.c_str());
        
        // Set last port if available
        if (!config.lastPort.empty()) {
            auto it = std::find(availablePorts_.begin(), availablePorts_.end(), config.lastPort);
            if (it != availablePorts_.end()) {
                selectedPortIndex_ = static_cast<int>(std::distance(availablePorts_.begin(), it));
            }
        }
        
        // Set last baud rate
        auto it = std::find(baudRates_.begin(), baudRates_.end(), config.lastBaudRate);
        if (it != baudRates_.end()) {
            selectedBaudRate_ = static_cast<int>(std::distance(baudRates_.begin(), it));
        }
    }
}

void SerialApp::saveConfiguration() {
    auto& config = configManager_.getConfig();
    
    // Save EXT commands
    for (int tab = 0; tab < 3; ++tab) {
        auto* commands = (tab == 0) ? &config.ext1Commands : 
                        (tab == 1) ? &config.ext2Commands : &config.ext3Commands;
        
        for (int i = 0; i < EXT_COMMAND_COUNT; ++i) {
            (*commands)[i].command = std::string(extCommands_[tab][i]);
            (*commands)[i].name = std::string(extNames_[tab][i]);
        }
    }
    
    // Save toggle commands
    config.toggleCommand0 = std::string(toggleCommand0_);
    config.toggleCommand1 = std::string(toggleCommand1_);
    config.toggleInterval0 = toggleInterval0_;
    config.toggleInterval1 = toggleInterval1_;
    
    // Save filter
    config.filterString = std::string(filterBuffer_);
    
    configManager_.saveConfig();
}

bool SerialApp::splitterV(const char* str_id, float* size1, float* size2, float min_size1, float min_size2) {
    ImGui::PushID(str_id);
    
    // Create an invisible button as a splitter
    ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
    ImVec2 splitter_size = ImVec2(ImGui::GetContentRegionAvail().x, 8.0f);
    
    // Make the splitter area more visible during hover/drag
    bool hovered = false;
    bool held = false;
    ImGui::InvisibleButton("splitter", splitter_size);
    
    if (ImGui::IsItemActive() || ImGui::IsItemHovered()) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        hovered = true;
    }
    
    if (ImGui::IsItemActive()) {
        held = true;
        float delta = ImGui::GetIO().MouseDelta.y;
        if (delta != 0.0f) {
            *size1 += delta;
            *size2 -= delta;
            
            // Apply constraints
            if (*size1 < min_size1) {
                float diff = min_size1 - *size1;
                *size1 = min_size1;
                *size2 -= diff;
            }
            if (*size2 < min_size2) {
                float diff = min_size2 - *size2;
                *size2 = min_size2;
                *size1 -= diff;
            }
        }
    }
    
    // Draw the splitter line
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImU32 col = ImGui::GetColorU32(hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Border);
    draw_list->AddLine(
        ImVec2(cursor_pos.x, cursor_pos.y + splitter_size.y / 2),
        ImVec2(cursor_pos.x + splitter_size.x, cursor_pos.y + splitter_size.y / 2),
        col, 2.0f
    );
    
    ImGui::PopID();
    return held;
}

void SerialApp::startLogging() {
    if (!logFile_.is_open()) {
        logFile_.open(logFilePath_, std::ios::app);
        if (logFile_.is_open()) {
            std::string startMsg = "=== Logging started ===";
            if (timestampEnabled_) {
                startMsg = getCurrentTimestamp() + " " + startMsg;
            }
            logFile_ << startMsg << std::endl;
            logFile_.flush();
        }
    }
}

void SerialApp::stopLogging() {
    if (logFile_.is_open()) {
        std::string stopMsg = "=== Logging stopped ===";
        if (timestampEnabled_) {
            stopMsg = getCurrentTimestamp() + " " + stopMsg;
        }
        logFile_ << stopMsg << std::endl;
        logFile_.flush();
        logFile_.close();
    }
}

void SerialApp::logData(const std::string& data, bool isReceived) {
    if (logFile_.is_open() && loggingEnabled_) {
        std::string prefix = isReceived ? "[RX] " : "[TX] ";
        std::string logEntry = prefix + data;
        
        if (timestampEnabled_) {
            logEntry = getCurrentTimestamp() + " " + logEntry;
        }
        
        logFile_ << logEntry;
        if (data.back() != '\n') {
            logFile_ << std::endl;
        }
        logFile_.flush();
    }
}

std::string SerialApp::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

std::string SerialApp::generateAutoFilename() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << "com_log_" << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S") << ".txt";
    return ss.str();
}

void SerialApp::openFileDialog() {
#ifdef _WIN32
    // Windows file dialog using Windows API
    char filename[512] = "";
    
    OPENFILENAMEA ofn;
    ZeroMemory(&filename, sizeof(filename));
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "Text Files\0*.txt\0Log Files\0*.log\0All Files\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = sizeof(filename);
    ofn.lpstrTitle = "Save Log File As...";
    ofn.Flags = OFN_DONTADDTORECENT | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT;
    
    if (GetSaveFileNameA(&ofn)) {
        strcpy(logFilePath_, filename);
    }
#else
    // For non-Windows platforms, provide a simple input dialog
    static char tempPath[512] = "";
    strcpy(tempPath, logFilePath_);
    
    // This would need a proper file dialog implementation for Linux/Mac
    // For now, just copy current path to temp for editing
    // You could integrate with libraries like nativefiledialog or tinyfiledialogs here
    strcpy(logFilePath_, "logs/com_log.txt"); // Default fallback
#endif
}