#include "ConfigManager.h"
#include <fstream>
#include <iostream>

AppConfig::AppConfig() {
    initializeDefaults();
}

void AppConfig::initializeDefaults() {
    ext1Groups = { ExtGroup("Group 1") };
    ext2Groups = { ExtGroup("Group 1") };
    ext3Groups = { ExtGroup("Group 1") };
}

ConfigManager::ConfigManager(const std::string& configFile)
    : configFile_(configFile) {
}

bool ConfigManager::loadConfig() {
    std::ifstream file(configFile_);
    if (!file.is_open()) {
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

void ConfigManager::to_json(nlohmann::json& j, const ExtGroup& group) {
    j["name"] = group.name;
    j["commands"] = nlohmann::json::array();
    for (const auto& cmd : group.commands) {
        nlohmann::json cmdJson;
        to_json(cmdJson, cmd);
        j["commands"].push_back(cmdJson);
    }
}

void ConfigManager::from_json(const nlohmann::json& j, ExtGroup& group) {
    j.at("name").get_to(group.name);
    if (j.contains("commands")) {
        for (const auto& cmdJson : j["commands"]) {
            if (cmdJson.is_object()) {
                ExtCommand cmd;
                from_json(cmdJson, cmd);
                group.commands.push_back(cmd);
            }
        }
    }
}

// Migrate old flat command list (v1 format) into a single default group
static void migrateOldCommands(const nlohmann::json& j, const std::string& key,
                                std::vector<ExtGroup>& groups) {
    ExtGroup defaultGroup("Default");
    for (const auto& cmdJson : j[key]) {
        if (cmdJson.is_object()) {
            std::string cmd  = cmdJson.value("command", "");
            std::string name = cmdJson.value("name", "");
            if (!cmd.empty()) {
                defaultGroup.commands.push_back(
                    ExtCommand(name.empty() ? "Button" : name, cmd));
            }
        }
    }
    if (!defaultGroup.commands.empty()) {
        groups.push_back(defaultGroup);
    }
    if (groups.empty()) {
        groups.push_back(ExtGroup("Group 1"));
    }
}

void ConfigManager::to_json(nlohmann::json& j, const AppConfig& config) {
    auto serializeGroups = [&](const std::string& key,
                               const std::vector<ExtGroup>& groups) {
        j[key] = nlohmann::json::array();
        for (const auto& grp : groups) {
            nlohmann::json grpJson;
            to_json(grpJson, grp);
            j[key].push_back(grpJson);
        }
    };

    serializeGroups("ext1Groups", config.ext1Groups);
    serializeGroups("ext2Groups", config.ext2Groups);
    serializeGroups("ext3Groups", config.ext3Groups);

    j["toggleCommand0"] = config.toggleCommand0;
    j["toggleCommand1"] = config.toggleCommand1;
    j["toggleInterval0"] = config.toggleInterval0;
    j["toggleInterval1"] = config.toggleInterval1;
    j["lastPort"]        = config.lastPort;
    j["lastBaudRate"]    = config.lastBaudRate;
    j["filterString"]    = config.filterString;
    j["filterActive"]    = config.filterActive;
}

void ConfigManager::from_json(const nlohmann::json& j, AppConfig& config) {
    config.initializeDefaults();

    auto loadGroups = [&](const std::string& newKey, const std::string& oldKey,
                          std::vector<ExtGroup>& groups) {
        if (j.contains(newKey)) {
            groups.clear();
            for (const auto& grpJson : j[newKey]) {
                if (grpJson.is_object()) {
                    ExtGroup grp;
                    from_json(grpJson, grp);
                    groups.push_back(grp);
                }
            }
            if (groups.empty()) groups.push_back(ExtGroup("Group 1"));
        } else if (j.contains(oldKey)) {
            groups.clear();
            migrateOldCommands(j, oldKey, groups);
        }
    };

    loadGroups("ext1Groups", "ext1Commands", config.ext1Groups);
    loadGroups("ext2Groups", "ext2Commands", config.ext2Groups);
    loadGroups("ext3Groups", "ext3Commands", config.ext3Groups);

    if (j.contains("toggleCommand0")) j["toggleCommand0"].get_to(config.toggleCommand0);
    if (j.contains("toggleCommand1")) j["toggleCommand1"].get_to(config.toggleCommand1);
    if (j.contains("toggleInterval0")) j["toggleInterval0"].get_to(config.toggleInterval0);
    if (j.contains("toggleInterval1")) j["toggleInterval1"].get_to(config.toggleInterval1);
    if (j.contains("lastPort"))     j["lastPort"].get_to(config.lastPort);
    if (j.contains("lastBaudRate")) j["lastBaudRate"].get_to(config.lastBaudRate);
    if (j.contains("filterString")) j["filterString"].get_to(config.filterString);
    if (j.contains("filterActive")) j["filterActive"].get_to(config.filterActive);
}
