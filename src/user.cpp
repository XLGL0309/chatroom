#include "../include/user.h"

bool UserManager::addUser(const std::string& username, const std::string& ip) {
    std::lock_guard<std::mutex> lock(userMutex);
    auto it = userMap.find(username);
    if (it == userMap.end()) {
        // 用户名不存在，添加新用户
        User user;
        user.username = username;
        user.ip = ip;
        userMap[username] = user;
        return true;
    } else {
        // 用户名已存在，检查IP是否匹配
        if (it->second.ip == ip) {
            // IP匹配，允许登录
            return true;
        } else {
            // IP不匹配，拒绝登录
            return false;
        }
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