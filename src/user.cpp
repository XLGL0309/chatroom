#include "../include/user.h"

bool UserManager::registerUser(const std::string& username, const std::string& password, const std::string& ip) {
    std::lock_guard<std::mutex> lock(userMutex);
    auto it = userMap.find(username);
    if (it == userMap.end()) {
        // 用户名不存在，注册新用户
        User user;
        user.username = username;
        user.password = password; // 简单存储密码，实际应用中应该加密
        user.ip = ip;
        userMap[username] = user;
        return true;
    } else {
        // 用户名已存在，注册失败
        return false;
    }
}

bool UserManager::loginUser(const std::string& username, const std::string& password, const std::string& ip) {
    std::lock_guard<std::mutex> lock(userMutex);
    auto it = userMap.find(username);
    if (it != userMap.end() && it->second.password == password) {
        // 用户名和密码匹配，更新IP并登录
        it->second.ip = ip;
        return true;
    } else {
        // 用户名或密码错误，登录失败
        return false;
    }
}

bool UserManager::isValidUser(const std::string& username, const std::string& ip) {
    std::lock_guard<std::mutex> lock(userMutex);
    auto it = userMap.find(username);
    return (it != userMap.end() && it->second.ip == ip);
}

bool UserManager::userExists(const std::string& username) {
    std::lock_guard<std::mutex> lock(userMutex);
    return userMap.find(username) != userMap.end();
}

// 全局用户管理器实例
UserManager g_userManager;