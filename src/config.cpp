/*
 * config.cpp
 * 配置管理功能的实现文件
 * 功能：实现配置管理器的方法，用于加载和获取配置信息
 */

#include "../include/config.h"
#include <iostream>
#include <fstream>
#include <sstream>

/**
 * 构造函数
 * 功能：初始化配置管理器并加载配置文件
 * 参数：file - 配置文件路径
 */
ConfigManager::ConfigManager(const std::string& file) : configFile(file) {
    load();
}

/**
 * 析构函数
 */
ConfigManager::~ConfigManager() {
}

/**
 * 加载配置文件
 * 功能：从配置文件中读取配置信息
 * 返回值：成功返回true，失败返回false
 */
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

/**
 * 获取字符串配置
 * 功能：根据键获取配置值
 * 参数：key - 配置键名
 *       defaultValue - 默认值，当配置不存在时返回
 * 返回值：配置值或默认值
 */
std::string ConfigManager::get(const std::string& key, const std::string& defaultValue) {
    auto it = configMap.find(key);
    if (it != configMap.end()) {
        return it->second;
    }
    return defaultValue;
}

/**
 * 获取整数配置
 * 功能：根据键获取配置值并转换为整数
 * 参数：key - 配置键名
 *       defaultValue - 默认值，当配置不存在或转换失败时返回
 * 返回值：配置值或默认值
 */
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

/**
 * 获取单例实例
 * 功能：获取配置管理器的单例实例
 * 参数：file - 配置文件路径
 * 返回值：ConfigManager的引用
 */
ConfigManager& ConfigManager::getInstance(const std::string& file) {
    static ConfigManager instance(file);
    return instance;
}