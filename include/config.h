#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>

class ConfigManager {
private:
    std::map<std::string, std::string> configMap;
    std::string configFile;

public:
    ConfigManager(const std::string& file = "config.ini");
    ~ConfigManager();
    
    bool load();
    std::string get(const std::string& key, const std::string& defaultValue = "");
    int getInt(const std::string& key, int defaultValue = 0);
};

// 全局配置管理器实例
extern ConfigManager g_configManager;

#endif // CONFIG_H