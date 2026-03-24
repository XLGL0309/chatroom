#ifndef USER_H
#define USER_H

#include <string>
#include <map>
#include <mutex>

class UserManager {
private:
    // 私有构造函数，防止外部实例化
    UserManager();
    ~UserManager();
    
    // 禁止复制和赋值
    UserManager(const UserManager&) = delete;
    UserManager& operator=(const UserManager&) = delete;

public:
    bool registerUser(const std::string& username, const std::string& password);
    bool loginUser(const std::string& username, const std::string& password);
    bool userExists(const std::string& username);
    
    // 静态方法获取单例实例
    static UserManager& getInstance();
};

#endif // USER_H