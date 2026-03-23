#ifndef USER_H
#define USER_H

#include <string>
#include <map>
#include <mutex>

struct User {
    std::string username;
    std::string password;
};

class UserManager {
private:

public:
    bool registerUser(const std::string& username, const std::string& password);
    bool loginUser(const std::string& username, const std::string& password);
    bool userExists(const std::string& username);
};

// 全局用户管理器实例
extern UserManager g_userManager;

#endif // USER_H