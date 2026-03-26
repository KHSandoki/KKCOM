#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct ExtCommand {
    std::string name;
    std::string command;

    ExtCommand() = default;
    ExtCommand(const std::string& n, const std::string& c) : name(n), command(c) {}
};

struct ExtGroup {
    std::string name;
    std::vector<ExtCommand> commands;

    ExtGroup() = default;
    ExtGroup(const std::string& n) : name(n) {}
};

struct AppConfig {
    std::vector<ExtGroup> ext1Groups;
    std::vector<ExtGroup> ext2Groups;
    std::vector<ExtGroup> ext3Groups;

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
