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
    void to_json(nlohmann::json& j, const AppConfig& config);
    void from_json(const nlohmann::json& j, AppConfig& config);
};
