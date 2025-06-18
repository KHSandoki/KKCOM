#include "ConfigManager.h"
#include <fstream>
#include <iostream>

AppConfig::AppConfig() {
    initializeDefaults();
}

void AppConfig::initializeDefaults() {
    ext1Commands.resize(100);
    ext2Commands.resize(100);
    ext3Commands.resize(100);
    
    for (int i = 0; i < 100; ++i) {
        ext1Commands[i] = ExtCommand(std::to_string(i), "");
        ext2Commands[i] = ExtCommand(std::to_string(i), "");
        ext3Commands[i] = ExtCommand(std::to_string(i), "");
    }
}

ConfigManager::ConfigManager(const std::string& configFile) 
    : configFile_(configFile) {
}

bool ConfigManager::loadConfig() {
    std::ifstream file(configFile_);
    if (!file.is_open()) {
        // Create default config if file doesn't exist
        config_.initializeDefaults();
        return saveConfig();
    }
    
    try {
        nlohmann::json j;
        file >> j;
        from_json(j, config_);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
        config_.initializeDefaults();
        return false;
    }
}

bool ConfigManager::saveConfig() {
    try {
        nlohmann::json j;
        to_json(j, config_);
        
        std::ofstream file(configFile_);
        if (!file.is_open()) {
            return false;
        }
        
        file << j.dump(4);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving config: " << e.what() << std::endl;
        return false;
    }
}

void ConfigManager::to_json(nlohmann::json& j, const ExtCommand& cmd) {
    j = nlohmann::json{
        {"name", cmd.name},
        {"command", cmd.command}
    };
}

void ConfigManager::from_json(const nlohmann::json& j, ExtCommand& cmd) {
    j.at("name").get_to(cmd.name);
    j.at("command").get_to(cmd.command);
}

void ConfigManager::to_json(nlohmann::json& j, const AppConfig& config) {
    // Manually serialize ExtCommand vectors
    j["ext1Commands"] = nlohmann::json::array();
    for (const auto& cmd : config.ext1Commands) {
        nlohmann::json cmdJson;
        to_json(cmdJson, cmd);
        j["ext1Commands"].push_back(cmdJson);
    }
    
    j["ext2Commands"] = nlohmann::json::array();
    for (const auto& cmd : config.ext2Commands) {
        nlohmann::json cmdJson;
        to_json(cmdJson, cmd);
        j["ext2Commands"].push_back(cmdJson);
    }
    
    j["ext3Commands"] = nlohmann::json::array();
    for (const auto& cmd : config.ext3Commands) {
        nlohmann::json cmdJson;
        to_json(cmdJson, cmd);
        j["ext3Commands"].push_back(cmdJson);
    }
    
    j["toggleCommand0"] = config.toggleCommand0;
    j["toggleCommand1"] = config.toggleCommand1;
    j["toggleInterval0"] = config.toggleInterval0;
    j["toggleInterval1"] = config.toggleInterval1;
    j["lastPort"] = config.lastPort;
    j["lastBaudRate"] = config.lastBaudRate;
    j["filterString"] = config.filterString;
    j["filterActive"] = config.filterActive;
}

void ConfigManager::from_json(const nlohmann::json& j, AppConfig& config) {
    config.initializeDefaults();
    
    if (j.contains("ext1Commands")) {
        auto& cmds = j["ext1Commands"];
        for (size_t i = 0; i < std::min(cmds.size(), config.ext1Commands.size()); ++i) {
            if (cmds[i].is_object()) {
                from_json(cmds[i], config.ext1Commands[i]);
            }
        }
    }
    
    if (j.contains("ext2Commands")) {
        auto& cmds = j["ext2Commands"];
        for (size_t i = 0; i < std::min(cmds.size(), config.ext2Commands.size()); ++i) {
            if (cmds[i].is_object()) {
                from_json(cmds[i], config.ext2Commands[i]);
            }
        }
    }
    
    if (j.contains("ext3Commands")) {
        auto& cmds = j["ext3Commands"];
        for (size_t i = 0; i < std::min(cmds.size(), config.ext3Commands.size()); ++i) {
            if (cmds[i].is_object()) {
                from_json(cmds[i], config.ext3Commands[i]);
            }
        }
    }
    
    if (j.contains("toggleCommand0")) j["toggleCommand0"].get_to(config.toggleCommand0);
    if (j.contains("toggleCommand1")) j["toggleCommand1"].get_to(config.toggleCommand1);
    if (j.contains("toggleInterval0")) j["toggleInterval0"].get_to(config.toggleInterval0);
    if (j.contains("toggleInterval1")) j["toggleInterval1"].get_to(config.toggleInterval1);
    if (j.contains("lastPort")) j["lastPort"].get_to(config.lastPort);
    if (j.contains("lastBaudRate")) j["lastBaudRate"].get_to(config.lastBaudRate);
    if (j.contains("filterString")) j["filterString"].get_to(config.filterString);
    if (j.contains("filterActive")) j["filterActive"].get_to(config.filterActive);
}