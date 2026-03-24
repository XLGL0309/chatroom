#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <vector>
#include <mutex>
#include <map>

struct Message {
    std::string from;
    std::string to;
    std::string content;
    time_t timestamp; // 添加时间戳，用于清理过期消息

    Message();
};

class MessageManager {
private:
    // 私有构造函数，防止外部实例化
    MessageManager();
    ~MessageManager();
    
    // 禁止复制和赋值
    MessageManager(const MessageManager&) = delete;
    MessageManager& operator=(const MessageManager&) = delete;

public:
    void addMessage(const std::string& from, const std::string& to, const std::string& content);
    std::vector<Message> getMessagesForUser(const std::string& username);
    void cleanExpiredMessages();
    void cleanExpiredMessagesForUser(const std::string& username);
    
    // 静态方法获取单例实例
    static MessageManager& getInstance();
};

#endif // MESSAGE_H