#include "../include/config.h"
#include <iostream>
#include <fstream>
#include <sstream>

ConfigManager::ConfigManager(const std::string& file) : configFile(file) {
    load();
}

ConfigManager::~ConfigManager() {
}

bool ConfigManager::load() {
    std::ifstream file(configFile);
    if (!file.is_open()) {
        std::cerr << "Config file not found: " << configFile << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        // 跳过空行和注释
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // 解析键值对
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            // 去除空格
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            configMap[key] = value;
        }
    }

    file.close();
    return true;
}

std::string ConfigManager::get(const std::string& key, const std::string& defaultValue) {
    auto it = configMap.find(key);
    if (it != configMap.end()) {
        return it->second;
    }
    return defaultValue;
}

int ConfigManager::getInt(const std::string& key, int defaultValue) {
    auto it = configMap.find(key);
    if (it != configMap.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

// 静态方法获取单例实例
ConfigManager& ConfigManager::getInstance(const std::string& file) {
    static ConfigManager instance(file);
    return instance;
}