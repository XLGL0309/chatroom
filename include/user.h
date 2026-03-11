#ifndef USER_H
#define USER_H

#include <string>
#include <map>
#include <mutex>

struct User {
    std::string username;
    std::string password;
    std::string ip;
};

class UserManager {
private:
    std::map<std::string, User> userMap;
    std::mutex userMutex;

public:
    bool registerUser(const std::string& username, const std::string& password, const std::string& ip);
    bool loginUser(const std::string& username, const std::string& password, const std::string& ip);
    bool isValidUser(const std::string& username, const std::string& ip);
    bool userExists(const std::string& username);
};

// 全局用户管理器实例
extern UserManager g_userManager;

#endif // USER_H