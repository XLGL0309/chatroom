/*
 * user.h
 * 用户管理功能的头文件
 * 功能：定义用户管理器类，用于用户注册、登录和检查
 */

#ifndef USER_H
#define USER_H

#include <string>
#include <map>
#include <mutex>

/**
 * 用户管理器类
 * 功能：使用单例模式管理用户
 * 说明：采用懒汉式单例模式，确保线程安全
 */
class UserManager {
private:
    /**
     * 私有构造函数
     * 功能：初始化用户管理器
     */
    UserManager();
    
    /**
     * 析构函数
     */
    ~UserManager();
    
    // 禁止复制和赋值
    UserManager(const UserManager&) = delete;
    UserManager& operator=(const UserManager&) = delete;

public:
    /**
     * 注册用户
     * 功能：注册新用户
     * 参数：username - 用户名
     *       password - 密码
     * 返回值：成功返回true，失败返回false
     */
    bool registerUser(const std::string& username, const std::string& password);
    
    /**
     * 用户登录
     * 功能：验证用户登录信息
     * 参数：username - 用户名
     *       password - 密码
     * 返回值：成功返回true，失败返回false
     */
    bool loginUser(const std::string& username, const std::string& password);
    
    /**
     * 检查用户是否存在
     * 功能：检查指定用户名是否存在
     * 参数：username - 用户名
     * 返回值：存在返回true，不存在返回false
     */
    bool userExists(const std::string& username);
    
    /**
     * 获取单例实例
     * 功能：获取用户管理器的单例实例
     * 返回值：UserManager的引用
     */
    static UserManager& getInstance();
};

#endif // USER_H