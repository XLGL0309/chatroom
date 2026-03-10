#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <vector>
#include <mutex>

struct Message {
    std::string from;
    std::string to;
    std::string content;
    time_t timestamp; // 添加时间戳，用于清理过期消息

    Message();
};

class MessageManager {
private:
    std::vector<Message> messageList;
    std::mutex messageMutex;

public:
    void addMessage(const std::string& from, const std::string& to, const std::string& content);
    std::vector<Message> getMessagesForUser(const std::string& username);
    void cleanExpiredMessages();
};

// 全局消息管理器实例
extern MessageManager g_messageManager;

#endif // MESSAGE_H