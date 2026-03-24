/*
 * config.h
 * 配置管理功能的头文件
 * 功能：定义配置管理器类，用于加载和获取配置信息
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>

/**
 * 配置管理器类
 * 功能：使用单例模式管理配置文件
 * 说明：采用懒汉式单例模式，确保线程安全
 */
class ConfigManager {
private:
    // 配置键值对映射
    std::map<std::string, std::string> configMap;
    // 配置文件路径
    std::string configFile;
    
    /**
     * 私有构造函数
     * 功能：初始化配置管理器
     * 参数：file - 配置文件路径，默认为"config.ini"
     */
    ConfigManager(const std::string& file = "config.ini");
    
    /**
     * 析构函数
     */
    ~ConfigManager();
    
    // 禁止复制和赋值
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

public:
    /**
     * 加载配置文件
     * 功能：从配置文件中读取配置信息
     * 返回值：成功返回true，失败返回false
     */
    bool load();
    
    /**
     * 获取字符串配置
     * 功能：根据键获取配置值
     * 参数：key - 配置键名
     *       defaultValue - 默认值，当配置不存在时返回
     * 返回值：配置值或默认值
     */
    std::string get(const std::string& key, const std::string& defaultValue = "");
    
    /**
     * 获取整数配置
     * 功能：根据键获取配置值并转换为整数
     * 参数：key - 配置键名
     *       defaultValue - 默认值，当配置不存在或转换失败时返回
     * 返回值：配置值或默认值
     */
    int getInt(const std::string& key, int defaultValue = 0);
    
    /**
     * 获取单例实例
     * 功能：获取配置管理器的单例实例
     * 参数：file - 配置文件路径，默认为"config.ini"
     * 返回值：ConfigManager的引用
     */
    static ConfigManager& getInstance(const std::string& file = "config.ini");
};

#endif // CONFIG_H