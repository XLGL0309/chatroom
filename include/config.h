#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>

class ConfigManager {
private:
    std::map<std::string, std::string> configMap;
    std::string configFile;
    
    // 私有构造函数，防止外部实例化
    ConfigManager(const std::string& file = "config.ini");
    ~ConfigManager();
    
    // 禁止复制和赋值
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

public:
    bool load();
    std::string get(const std::string& key, const std::string& defaultValue = "");
    int getInt(const std::string& key, int defaultValue = 0);
    
    // 静态方法获取单例实例
    static ConfigManager& getInstance(const std::string& file = "config.ini");
};

#endif // CONFIG_H