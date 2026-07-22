#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct ExtCommand {
    std::string name;
    std::string command;
    float color[4];

    ExtCommand() : name(">"), command("") { color[0]=0.0f; color[1]=0.0f; color[2]=0.0f; color[3]=0.0f; }
    ExtCommand(const std::string& n, const std::string& c) : name(n), command(c) {
        color[0]=0.0f; color[1]=0.0f; color[2]=0.0f; color[3]=0.0f;
    }
};

struct ExtGroup {
    std::string name;
    std::vector<ExtCommand> commands;
    float color[4];

    ExtGroup() : name("") { color[0]=0.0f; color[1]=0.0f; color[2]=0.0f; color[3]=0.0f; }
    ExtGroup(const std::string& n) : name(n) { color[0]=0.0f; color[1]=0.0f; color[2]=0.0f; color[3]=0.0f; }
};

// A user-editable syntax-coloring rule for the Received Data view.
struct ColorRule {
    bool enabled = true;
    int type = 0;             // 0=Keywords, 1=Number, 2=Hex, 3=BracketTag
    std::string pattern;      // space/comma-separated words (type 0 only)
    float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};

    ColorRule() = default;
    ColorRule(int t, float r, float g, float b, const std::string& p = "")
        : type(t), pattern(p) { color[0]=r; color[1]=g; color[2]=b; color[3]=1.0f; }
};

struct AppConfig {
    std::vector<ExtGroup> ext1Groups;
    std::vector<ExtGroup> ext2Groups;
    std::vector<ExtGroup> ext3Groups;

    std::vector<ExtCommand> ext1PinnedCmds;
    std::vector<ExtCommand> ext2PinnedCmds;
    std::vector<ExtCommand> ext3PinnedCmds;

    std::string toggleCommand0;
    std::string toggleCommand1;
    int toggleInterval0 = 1;
    int toggleInterval1 = 1;

    std::string lastPort;
    int lastBaudRate = 115200;
    std::string filterString;
    bool filterActive = false;
    int lineEndingMode = 3;  // 0=None, 1=LF, 2=CR, 3=CR+LF (applied to command sends)

    // When true, KKCOM watches for a "release request" trigger file and, when it
    // appears, disconnects the serial port so another program (e.g. an automation
    // agent driving the same COM) can take it. Reconnect is manual.
    bool comReleaseWatch = true;

    bool syntaxColoring = false;       // master toggle for the Received Data view
    std::vector<ColorRule> colorRules; // user-editable coloring rules

    AppConfig();
    void initializeDefaults();
};

class ConfigManager {
public:
    ConfigManager(const std::string& configFile = "config.json");

    bool loadConfig();
    bool saveConfig();

    AppConfig& getConfig() { return config_; }
    const AppConfig& getConfig() const { return config_; }

private:
    std::string configFile_;
    AppConfig config_;

    void to_json(nlohmann::json& j, const ExtCommand& cmd);
    void from_json(const nlohmann::json& j, ExtCommand& cmd);
    void to_json(nlohmann::json& j, const ExtGroup& group);
    void from_json(const nlohmann::json& j, ExtGroup& group);
    void to_json(nlohmann::json& j, const ColorRule& rule);
    void from_json(const nlohmann::json& j, ColorRule& rule);
    void to_json(nlohmann::json& j, const AppConfig& config);
    void from_json(const nlohmann::json& j, AppConfig& config);
};
