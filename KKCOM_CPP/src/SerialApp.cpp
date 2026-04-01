#include "SerialApp.h"
#include "ResourceManager.h"
#include "DebugConfig.h"
#include "icon_data.h"
#include "launch_data.h"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
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

SerialApp::SerialApp() :
    configManager_("config.json"),
    textSelect_(
        [this](std::size_t i) -> std::string_view { return receivedData_[i]; },
        [this]() -> std::size_t { return receivedData_.size(); }
    )
{
    // Initialize buffers
    memset(inputBuffer_, 0, sizeof(inputBuffer_));
    memset(filterBuffer_, 0, sizeof(filterBuffer_));
    memset(toggleCommand0_, 0, sizeof(toggleCommand0_));
    memset(toggleCommand1_, 0, sizeof(toggleCommand1_));
    memset(tempEditName_, 0, sizeof(tempEditName_));
    memset(tempEditCmd_, 0, sizeof(tempEditCmd_));
    memset(tempGroupName_, 0, sizeof(tempGroupName_));
    memset(tempPinnedName_, 0, sizeof(tempPinnedName_));
    memset(tempPinnedCmd_, 0, sizeof(tempPinnedCmd_));
}

SerialApp::~SerialApp() {
    stopLogging();
    shutdown();
}

void SerialApp::showSplashScreen() {
    // Initialize GLFW for splash screen
    if (!glfwInit()) {
        DEBUG_ERROR("Failed to initialize GLFW for splash screen");
        return;
    }
    
    DEBUG_PRINT("Showing splash screen");
    
    // Show splash screen with embedded resources
    ResourceManager::showSplashScreen(image_launch_png, image_launch_png_len,
                                    image_icon_png, image_icon_png_len);
    
    // Clean up GLFW after splash screen
    glfwTerminate();
}

bool SerialApp::initialize() {
    DEBUG_PRINT("Initializing main application");
    
    // Initialize GLFW
    if (!glfwInit()) {
        DEBUG_ERROR("Failed to initialize GLFW");
        return false;
    }
    
    // Create window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    std::string mainWindowTitle = std::string(KKCOM_PRODUCT_NAME) + " v" + KKCOM_VERSION_STRING_FULL;
    GLFWwindow* window = glfwCreateWindow(1200, 800, mainWindowTitle.c_str(), nullptr, nullptr);
    if (!window) {
        DEBUG_ERROR("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }
    
    DEBUG_PRINT("Created main window successfully");
    
    // Set window icon
    setWindowIcon(window);
    
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

    // Auto-start logging with a generated filename to prevent overwriting
    {
        std::string autoName = generateAutoFilename();
        strcpy(logFilePath_, autoName.c_str());
        startLogging();
    }

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
    // Signal threads to stop and wake them immediately
    sendEveryRunning_ = false;
    sendEveryCv_.notify_all();
    toggleSendRunning_ = false;
    toggleSendCv_.notify_all();

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
    // Connection button (full width, no title)
    ImVec4 buttonColor = connected_ ? ImVec4(1.0f, 0.2f, 0.2f, 1.0f) : ImVec4(0.1f, 0.7f, 0.1f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    if (ImGui::Button(connected_ ? "Disconnect" : "Connect", ImVec2(-1, 40))) {
        toggleConnection();
    }
    ImGui::PopStyleColor();

    // COM port, refresh button, and baud rate on one line
    float avail = ImGui::GetContentRegionAvail().x;
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float refreshW = 28.0f;
    float comboW = (avail - refreshW - spacing * 2) * 0.55f;
    float baudW  = avail - comboW - refreshW - spacing * 2;

    ImGui::PushItemWidth(comboW);
    if (!availablePorts_.empty()) {
        std::vector<const char*> portCStrings;
        for (const auto& port : availablePorts_) {
            portCStrings.push_back(port.deviceName.c_str());
        }
        ImGui::Combo("##Port", &selectedPortIndex_, portCStrings.data(), static_cast<int>(portCStrings.size()));
    } else {
        ImGui::TextDisabled("(none)");
    }
    ImGui::PopItemWidth();

    ImGui::SameLine();
    if (ImGui::Button("R##Ref")) { refreshPorts(); }
    ImGui::SameLine();

    ImGui::PushItemWidth(baudW);
    const char* baudRateStrings[] = {"300","600","1200","2400","4800","9600","19200","38400","57600","115200","921600"};
    ImGui::Combo("##BaudRate", &selectedBaudRate_, baudRateStrings, IM_ARRAYSIZE(baudRateStrings));
    ImGui::PopItemWidth();
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
            sendEveryCv_.notify_all();
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
    // Drain receive queue into receivedData_ at the start of each frame
    drainPendingData();

    ImGui::Text("Received Data (Click to focus for single-key sending)");
    ImGui::Separator();

    // Apply scroll compensation BEFORE BeginChild so it takes effect in this frame's clipper.
    // SetNextWindowScroll is the only way to update scroll position same-frame in ImGui.
    if (!autoScroll_ && itemsRemovedFromFront_ > 0) {
        float lineHeight = ImGui::GetTextLineHeightWithSpacing();
        float targetScroll = prevScrollY_ - itemsRemovedFromFront_ * lineHeight;
        if (targetScroll < 0.0f) targetScroll = 0.0f;
        ImGui::SetNextWindowScroll(ImVec2(-1.0f, targetScroll));
    }
    itemsRemovedFromFront_ = 0;

    ImGui::BeginChild("DataScrolling", ImVec2(0, 0), true,
        ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove);

    // If this window is focused and no text is selected, capture character input for single-key sending
    if (ImGui::IsWindowFocused() && !textSelect_.hasSelection()) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.InputQueueCharacters.Size > 0) {
            for (int i = 0; i < io.InputQueueCharacters.Size; i++) {
                char c = io.InputQueueCharacters[i];
                if (c >= 32 || c == '\n' || c == '\r' || c == '\t') {
                    sendCommand(std::string(1, c));
                }
            }
            io.InputQueueCharacters.resize(0);
        }
    }

    // No mutex needed — receivedData_ and partialLine_ are render-thread only
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(receivedData_.size()));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
            ImGui::TextUnformatted(receivedData_[i].c_str());
        }
    }

    // Show incomplete line (no \n yet) so data is visible without waiting for newline
    if (!partialLine_.empty()) {
        ImGui::TextUnformatted(partialLine_.c_str());
    }

    // Smart auto-scroll: disable when user scrolls up, re-enable when back at bottom
    if (ImGui::IsWindowHovered() && ImGui::GetIO().MouseWheel > 0.0f) {
        autoScroll_ = false;
    }
    float scrollY    = ImGui::GetScrollY();
    float scrollMaxY = ImGui::GetScrollMaxY();
    if (scrollMaxY > 0.0f && scrollY >= scrollMaxY - 1.0f) {
        autoScroll_ = true;
    }
    if (autoScroll_) {
        ImGui::SetScrollHereY(1.0f);
        prevScrollY_ = scrollMaxY;
    } else {
        prevScrollY_ = scrollY;
    }

    // Only run TextSelect (which rebuilds a full-list vector) when the user
    // is actually interacting with the window — not every frame unconditionally.
    if (ImGui::IsWindowHovered() || ImGui::IsWindowFocused() || textSelect_.hasSelection()) {
        textSelect_.update();
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
    auto& groups = getTabGroups(tabIndex);
    auto& pinned = getTabPinnedCmds(tabIndex);

    ImGui::BeginChild("ExtCommands", ImVec2(0, 0), false);

    // --- Pinned quick-commands bar ---
    ImGui::PushID("PinnedBar");
    for (int pi = 0; pi < (int)pinned.size(); ++pi) {
        auto& pc = pinned[pi];
        ImGui::PushID(pi);
        if (pi > 0) ImGui::SameLine();

        bool hasColor = pc.color[3] > 0.01f;
        if (hasColor) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(pc.color[0], pc.color[1], pc.color[2], pc.color[3]));
            float luma = 0.2126f*pc.color[0] + 0.7152f*pc.color[1] + 0.0722f*pc.color[2];
            ImGui::PushStyleColor(ImGuiCol_Text, luma > 0.5f ? ImVec4(0,0,0,1) : ImVec4(1,1,1,1));
        }
        std::string plabel = pc.name.empty() ? (pc.command.empty() ? "?" : pc.command) : pc.name;
        if (ImGui::Button(plabel.c_str()) && !pc.command.empty())
            sendCommand(pc.command);
        if (hasColor) ImGui::PopStyleColor(2);

        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            contextPinnedTabIndex_ = tabIndex;
            contextPinnedIndex_    = pi;
            ImGui::OpenPopup("PinnedCtxMenu");
        }
        if (ImGui::BeginPopup("PinnedCtxMenu")) {
            if (ImGui::MenuItem("Edit")) {
                auto& p2 = getTabPinnedCmds(contextPinnedTabIndex_)[contextPinnedIndex_];
                pinnedEditTabIndex_ = contextPinnedTabIndex_;
                pinnedEditCmdIndex_ = contextPinnedIndex_;
                memset(tempPinnedName_, 0, sizeof(tempPinnedName_));
                strncpy(tempPinnedName_, p2.name.c_str(), sizeof(tempPinnedName_)-1);
                memset(tempPinnedCmd_, 0, sizeof(tempPinnedCmd_));
                strncpy(tempPinnedCmd_, p2.command.c_str(), sizeof(tempPinnedCmd_)-1);
                memcpy(tempPinnedColor_, p2.color, sizeof(float)*4);
                showPinnedEditPopup_ = true;
            }
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
            if (ImGui::MenuItem("Delete")) {
                auto& pv = getTabPinnedCmds(contextPinnedTabIndex_);
                if (contextPinnedIndex_ < (int)pv.size())
                    pv.erase(pv.begin() + contextPinnedIndex_);
            }
            ImGui::PopStyleColor();
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
    if (!pinned.empty()) ImGui::SameLine();
    if (ImGui::SmallButton("+##AddPinned")) {
        pinnedEditTabIndex_ = tabIndex;
        pinnedEditCmdIndex_ = -1;
        memset(tempPinnedName_, 0, sizeof(tempPinnedName_));
        memset(tempPinnedCmd_,  0, sizeof(tempPinnedCmd_));
        tempPinnedColor_[0]=0.26f; tempPinnedColor_[1]=0.59f;
        tempPinnedColor_[2]=0.98f; tempPinnedColor_[3]=0.80f;
        showPinnedEditPopup_ = true;
    }
    ImGui::PopID(); // PinnedBar

    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("+ Add Group")) {
        showAddGroupPopup_ = true;
        editTabIndex_ = tabIndex;
        memset(tempGroupName_, 0, sizeof(tempGroupName_));
        strncpy(tempGroupName_, "New Group", sizeof(tempGroupName_) - 1);
    }

    ImGui::Separator();
    ImGui::Spacing();

    int deleteGroupIdx = -1;

    for (int gi = 0; gi < (int)groups.size(); ++gi) {
        auto& group = groups[gi];
        ImGui::PushID(gi);

        bool hasGroupColor = group.color[3] > 0.01f;
        if (hasGroupColor) {
            ImVec4 hc(group.color[0], group.color[1], group.color[2], group.color[3]);
            ImGui::PushStyleColor(ImGuiCol_Header,        hc);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(hc.x*1.15f, hc.y*1.15f, hc.z*1.15f, hc.w));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive,  ImVec4(hc.x*0.85f, hc.y*0.85f, hc.z*0.85f, hc.w));
            float luma = 0.2126f*hc.x + 0.7152f*hc.y + 0.0722f*hc.z;
            ImGui::PushStyleColor(ImGuiCol_Text, luma > 0.5f ? ImVec4(0,0,0,1) : ImVec4(1,1,1,1));
        }
        bool open = ImGui::CollapsingHeader(group.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
        if (hasGroupColor) ImGui::PopStyleColor(4);

        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            contextTabIndex_   = tabIndex;
            contextGroupIndex_ = gi;
            ImGui::OpenPopup("GroupCtxMenu");
        }

        if (ImGui::BeginPopup("GroupCtxMenu")) {
            if (ImGui::MenuItem("Rename")) {
                showRenameGroupPopup_ = true;
                editTabIndex_   = contextTabIndex_;
                editGroupIndex_ = contextGroupIndex_;
                memset(tempGroupName_, 0, sizeof(tempGroupName_));
                strncpy(tempGroupName_, groups[contextGroupIndex_].name.c_str(),
                        sizeof(tempGroupName_) - 1);
            }
            if (ImGui::MenuItem("Change Color")) {
                editTabIndex_   = contextTabIndex_;
                editGroupIndex_ = contextGroupIndex_;
                memcpy(tempGroupColor_, groups[contextGroupIndex_].color, sizeof(float)*4);
                showGroupColorPopup_ = true;
            }
            if (ImGui::MenuItem("Add Command")) {
                groups[contextGroupIndex_].commands.push_back(ExtCommand(">", ""));
            }
            ImGui::Separator();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
            if (ImGui::MenuItem("Delete Group")) {
                deleteGroupIdx = contextGroupIndex_;
            }
            ImGui::PopStyleColor();
            ImGui::EndPopup();
        }

        if (open) {
            int deleteCmd = -1;
            int dragSrc = -1, dragDst = -1;

            ImGui::Columns(3, "CmdCols", true);
            ImGui::SetColumnWidth(0, 30.0f);
            ImGui::TextDisabled(" ");
            ImGui::NextColumn();
            ImGui::TextDisabled("Command");
            ImGui::NextColumn();
            ImGui::TextDisabled("Send  (right-click = edit)");
            ImGui::NextColumn();
            ImGui::Separator();

            for (int ci = 0; ci < (int)group.commands.size(); ++ci) {
                auto& cmd = group.commands[ci];
                ImGui::PushID(ci);

                // Drag handle column
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
                ImGui::SmallButton("=");
                ImGui::PopStyleColor(2);

                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                    ImGui::SetDragDropPayload("CMD_REORDER", &ci, sizeof(int));
                    ImGui::Text("Move: %s", cmd.name.c_str());
                    ImGui::EndDragDropSource();
                }
                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CMD_REORDER")) {
                        dragSrc = *(const int*)payload->Data;
                        dragDst = ci;
                    }
                    ImGui::EndDragDropTarget();
                }
                ImGui::NextColumn();

                // Command input column
                ImGui::PushItemWidth(-1);
                ImGui::InputText("##Cmd", &cmd.command);
                ImGui::PopItemWidth();
                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CMD_REORDER")) {
                        dragSrc = *(const int*)payload->Data;
                        dragDst = ci;
                    }
                    ImGui::EndDragDropTarget();
                }
                ImGui::NextColumn();

                // Send button — single-click sends, right-click opens edit
                bool hasCmdColor = cmd.color[3] > 0.01f;
                if (hasCmdColor) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(cmd.color[0], cmd.color[1], cmd.color[2], cmd.color[3]));
                    float luma = 0.2126f*cmd.color[0] + 0.7152f*cmd.color[1] + 0.0722f*cmd.color[2];
                    ImGui::PushStyleColor(ImGuiCol_Text, luma > 0.5f ? ImVec4(0,0,0,1) : ImVec4(1,1,1,1));
                }
                if (ImGui::Button(cmd.name.c_str()) && !cmd.command.empty())
                    sendCommand(cmd.command);
                if (hasCmdColor) ImGui::PopStyleColor(2);

                if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                    editTabIndex_     = tabIndex;
                    editGroupIndex_   = gi;
                    editCommandIndex_ = ci;
                    memset(tempEditName_, 0, sizeof(tempEditName_));
                    strncpy(tempEditName_, cmd.name.c_str(), sizeof(tempEditName_)-1);
                    memset(tempEditCmd_, 0, sizeof(tempEditCmd_));
                    strncpy(tempEditCmd_, cmd.command.c_str(), sizeof(tempEditCmd_)-1);
                    memcpy(tempEditColor_, cmd.color, sizeof(float)*4);
                    ImGui::OpenPopup("CmdCtxMenu");
                }
                if (ImGui::BeginPopup("CmdCtxMenu")) {
                    if (ImGui::MenuItem("Edit..."))
                        showEditWindow_ = true;
                    ImGui::EndPopup();
                }

                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CMD_REORDER")) {
                        dragSrc = *(const int*)payload->Data;
                        dragDst = ci;
                    }
                    ImGui::EndDragDropTarget();
                }

                // Delete button
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.5f, 0.1f, 0.1f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
                if (ImGui::SmallButton("x"))
                    deleteCmd = ci;
                ImGui::PopStyleColor(2);

                ImGui::NextColumn();
                ImGui::PopID();
            }

            ImGui::Columns(1);

            // Apply deferred drag reorder
            if (dragSrc >= 0 && dragDst >= 0 && dragSrc != dragDst) {
                if (dragSrc < dragDst) {
                    std::rotate(group.commands.begin() + dragSrc,
                                group.commands.begin() + dragSrc + 1,
                                group.commands.begin() + dragDst + 1);
                } else {
                    std::rotate(group.commands.begin() + dragDst,
                                group.commands.begin() + dragSrc,
                                group.commands.begin() + dragSrc + 1);
                }
            }

            if (deleteCmd >= 0)
                group.commands.erase(group.commands.begin() + deleteCmd);

            if (ImGui::Button("+ Add Command"))
                group.commands.push_back(ExtCommand(">", ""));

            ImGui::Spacing();
        }

        ImGui::PopID();
    }

    if (deleteGroupIdx >= 0 && deleteGroupIdx < (int)groups.size())
        groups.erase(groups.begin() + deleteGroupIdx);

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
            toggleSendCv_.notify_all();
            if (toggleSendThread_.joinable()) {
                toggleSendThread_.join();
            }
        }
    }
}

void SerialApp::onDataReceived(const std::string& data) {
    // Log on receive thread (no UI lock needed)
    logData(data, true);

    // Push to pending queue — minimal lock time
    std::lock_guard<std::mutex> lock(pendingMutex_);
    pendingData_.push(data);
}

void SerialApp::drainPendingData() {
    // Swap the pending queue out under minimal lock
    std::queue<std::string> local;
    {
        std::lock_guard<std::mutex> lock(pendingMutex_);
        std::swap(local, pendingData_);
    }

    // Split raw chunks into lines, maintaining partialLine_ for incomplete data.
    // No lock needed — receivedData_ and partialLine_ are render-thread only.
    const auto& config = configManager_.getConfig();
    static const size_t MAX_PARTIAL_LINE = 4096;

    auto pushLine = [&](std::string line) {
        if (config.filterActive && !config.filterString.empty()) {
            if (line.find(config.filterString) == std::string::npos)
                return;
        }
        receivedData_.push_back(std::move(line));
        if (receivedData_.size() > MAX_DISPLAY_LINES) {
            receivedData_.pop_front();
            ++itemsRemovedFromFront_;
        }
    };

    while (!local.empty()) {
        partialLine_ += std::move(local.front());
        local.pop();

        // Scan for newlines using an offset — avoids O(N^2) erase-in-loop.
        // All found lines are extracted in one pass; a single erase follows.
        size_t searchStart = 0;
        size_t pos;
        while ((pos = partialLine_.find('\n', searchStart)) != std::string::npos) {
            std::string line(partialLine_, searchStart, pos - searchStart);
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            line += '\n';
            searchStart = pos + 1;
            pushLine(std::move(line));
        }
        // Remove all consumed bytes in one shot
        if (searchStart > 0)
            partialLine_.erase(0, searchStart);

        // Cap partial line to avoid unbounded growth on no-newline streams
        while (partialLine_.size() > MAX_PARTIAL_LINE) {
            pushLine(partialLine_.substr(0, MAX_PARTIAL_LINE));
            partialLine_.erase(0, MAX_PARTIAL_LINE);
        }
    }
}

void SerialApp::renderEditWindow() {
    // --- Edit command button (name + command + color) ---
    if (showEditWindow_) {
        ImGui::OpenPopup("Edit Command Button");
        showEditWindow_ = false;
    }
    if (ImGui::BeginPopupModal("Edit Command Button", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Button name:");
        ImGui::SetNextItemWidth(300);
        ImGui::InputText("##BtnName", tempEditName_, sizeof(tempEditName_));
        ImGui::Text("Command:");
        ImGui::SetNextItemWidth(300);
        ImGui::InputText("##BtnCmd", tempEditCmd_, sizeof(tempEditCmd_));
        ImGui::Text("Button color (alpha=0 = default):");
        ImGui::ColorEdit4("##BtnColor", tempEditColor_,
            ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueWheel);
        ImGui::SameLine();
        if (ImGui::SmallButton("Reset##CmdClr")) {
            tempEditColor_[0]=0.0f; tempEditColor_[1]=0.0f;
            tempEditColor_[2]=0.0f; tempEditColor_[3]=0.0f;
        }
        if (ImGui::Button("OK")) {
            auto& groups = getTabGroups(editTabIndex_);
            if (editGroupIndex_ < (int)groups.size()) {
                auto& grp = groups[editGroupIndex_];
                if (editCommandIndex_ < (int)grp.commands.size()) {
                    grp.commands[editCommandIndex_].name    = std::string(tempEditName_);
                    grp.commands[editCommandIndex_].command = std::string(tempEditCmd_);
                    memcpy(grp.commands[editCommandIndex_].color, tempEditColor_, sizeof(float)*4);
                }
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    // --- Add group ---
    if (showAddGroupPopup_) {
        ImGui::OpenPopup("Add Group");
        showAddGroupPopup_ = false;
    }
    if (ImGui::BeginPopupModal("Add Group", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Group name:");
        ImGui::SetNextItemWidth(260);
        ImGui::InputText("##NewGroupName", tempGroupName_, sizeof(tempGroupName_));
        if (ImGui::Button("OK")) {
            getTabGroups(editTabIndex_).push_back(ExtGroup(std::string(tempGroupName_)));
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    // --- Rename group ---
    if (showRenameGroupPopup_) {
        ImGui::OpenPopup("Rename Group");
        showRenameGroupPopup_ = false;
    }
    if (ImGui::BeginPopupModal("Rename Group", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("New name:");
        ImGui::SetNextItemWidth(260);
        ImGui::InputText("##RenameGroupName", tempGroupName_, sizeof(tempGroupName_));
        if (ImGui::Button("OK")) {
            auto& groups = getTabGroups(editTabIndex_);
            if (editGroupIndex_ < (int)groups.size())
                groups[editGroupIndex_].name = std::string(tempGroupName_);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    // --- Change group color ---
    if (showGroupColorPopup_) {
        ImGui::OpenPopup("Change Group Color");
        showGroupColorPopup_ = false;
    }
    if (ImGui::BeginPopupModal("Change Group Color", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Header color (alpha=0 = default style):");
        ImGui::ColorPicker4("##GroupColor", tempGroupColor_,
            ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_AlphaBar);
        if (ImGui::Button("OK")) {
            auto& groups = getTabGroups(editTabIndex_);
            if (editGroupIndex_ < (int)groups.size())
                memcpy(groups[editGroupIndex_].color, tempGroupColor_, sizeof(float)*4);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset to Default")) {
            auto& groups = getTabGroups(editTabIndex_);
            if (editGroupIndex_ < (int)groups.size())
                memset(groups[editGroupIndex_].color, 0, sizeof(float)*4);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    // --- Edit pinned command ---
    if (showPinnedEditPopup_) {
        ImGui::OpenPopup("Edit Pinned Command");
        showPinnedEditPopup_ = false;
    }
    if (ImGui::BeginPopupModal("Edit Pinned Command", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Button name:");
        ImGui::SetNextItemWidth(300);
        ImGui::InputText("##PinnedName", tempPinnedName_, sizeof(tempPinnedName_));
        ImGui::Text("Command:");
        ImGui::SetNextItemWidth(300);
        ImGui::InputText("##PinnedCmd", tempPinnedCmd_, sizeof(tempPinnedCmd_));
        ImGui::Text("Button color:");
        ImGui::ColorEdit4("##PinnedColor", tempPinnedColor_,
            ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueWheel);
        if (ImGui::Button("OK")) {
            auto& pv = getTabPinnedCmds(pinnedEditTabIndex_);
            if (pinnedEditCmdIndex_ < 0) {
                ExtCommand pc;
                pc.name    = std::string(tempPinnedName_);
                pc.command = std::string(tempPinnedCmd_);
                memcpy(pc.color, tempPinnedColor_, sizeof(float)*4);
                pv.push_back(pc);
            } else if (pinnedEditCmdIndex_ < (int)pv.size()) {
                pv[pinnedEditCmdIndex_].name    = std::string(tempPinnedName_);
                pv[pinnedEditCmdIndex_].command  = std::string(tempPinnedCmd_);
                memcpy(pv[pinnedEditCmdIndex_].color, tempPinnedColor_, sizeof(float)*4);
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();
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
            const std::string& selectedPort = availablePorts_[selectedPortIndex_].port;
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
        if (strlen(inputBuffer_) > 0)
            sendCommand(std::string(inputBuffer_));

        std::unique_lock<std::mutex> lk(sendEveryMutex_);
        sendEveryCv_.wait_for(lk,
            std::chrono::seconds(sendEveryInterval_),
            [this] { return !sendEveryRunning_.load(); });
    }
}

void SerialApp::toggleSendLoop() {
    bool sendFirst = true;

    while (toggleSendRunning_) {
        int intervalSec;
        if (sendFirst) {
            if (strlen(toggleCommand0_) > 0)
                sendCommand(std::string(toggleCommand0_));
            intervalSec = toggleInterval0_;
        } else {
            if (strlen(toggleCommand1_) > 0)
                sendCommand(std::string(toggleCommand1_));
            intervalSec = toggleInterval1_;
        }
        sendFirst = !sendFirst;

        std::unique_lock<std::mutex> lk(toggleSendMutex_);
        toggleSendCv_.wait_for(lk,
            std::chrono::seconds(intervalSec),
            [this] { return !toggleSendRunning_.load(); });
    }
}

void SerialApp::clearDataDisplay() {
    // Clear pending queue under its own lock
    {
        std::lock_guard<std::mutex> lock(pendingMutex_);
        std::queue<std::string> empty;
        std::swap(pendingData_, empty);
    }
    // receivedData_ and partialLine_ are render-thread only, no lock needed
    receivedData_.clear();
    partialLine_.clear();
    textSelect_.clearSelection();
}

void SerialApp::applyFilter() {
    auto& config = configManager_.getConfig();
    config.filterString = std::string(filterBuffer_);
    config.filterActive = !config.filterString.empty();
}

void SerialApp::loadConfiguration() {
    if (configManager_.loadConfig()) {
        const auto& config = configManager_.getConfig();

        // Load toggle commands
        strncpy(toggleCommand0_, config.toggleCommand0.c_str(), sizeof(toggleCommand0_) - 1);
        strncpy(toggleCommand1_, config.toggleCommand1.c_str(), sizeof(toggleCommand1_) - 1);
        toggleInterval0_ = config.toggleInterval0;
        toggleInterval1_ = config.toggleInterval1;

        // Load filter
        strncpy(filterBuffer_, config.filterString.c_str(), sizeof(filterBuffer_) - 1);

        // Set last port if available
        if (!config.lastPort.empty()) {
            for (size_t i = 0; i < availablePorts_.size(); ++i) {
                if (availablePorts_[i].port == config.lastPort) {
                    selectedPortIndex_ = static_cast<int>(i);
                    break;
                }
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

    // EXT groups are edited directly in config — no sync needed

    // Save toggle commands
    config.toggleCommand0 = std::string(toggleCommand0_);
    config.toggleCommand1 = std::string(toggleCommand1_);
    config.toggleInterval0 = toggleInterval0_;
    config.toggleInterval1 = toggleInterval1_;

    // Save filter
    config.filterString = std::string(filterBuffer_);

    configManager_.saveConfig();
}

std::vector<ExtGroup>& SerialApp::getTabGroups(int tabIndex) {
    auto& config = configManager_.getConfig();
    if (tabIndex == 0) return config.ext1Groups;
    if (tabIndex == 1) return config.ext2Groups;
    return config.ext3Groups;
}

std::vector<ExtCommand>& SerialApp::getTabPinnedCmds(int tabIndex) {
    auto& config = configManager_.getConfig();
    if (tabIndex == 0) return config.ext1PinnedCmds;
    if (tabIndex == 1) return config.ext2PinnedCmds;
    return config.ext3PinnedCmds;
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
        logFile_ << stopMsg << '\n';
        logFile_.flush();
        logFlushCounter_ = 0;
        logFile_.close();
    }
}

void SerialApp::logData(const std::string& data, bool isReceived) {
    if (!logFile_.is_open() || !loggingEnabled_) return;

    std::string prefix = isReceived ? "[RX] " : "[TX] ";
    std::string logEntry = prefix + data;

    if (timestampEnabled_) {
        logEntry = getCurrentTimestamp() + " " + logEntry;
    }

    logFile_ << logEntry;
    if (data.back() != '\n') {
        logFile_ << '\n';
    }

    // Flush every 50 lines instead of every line to avoid per-line disk I/O
    if (++logFlushCounter_ >= 50) {
        logFile_.flush();
        logFlushCounter_ = 0;
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

void SerialApp::setWindowIcon(GLFWwindow* window) {
    ResourceManager::setWindowIcon(window, image_icon_png, image_icon_png_len);
}