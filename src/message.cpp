#include "../include/message.h"
#include <iostream>

Message::Message() : timestamp(time(nullptr)) {}

void MessageManager::addMessage(const std::string& from, const std::string& to, const std::string& content) {
    std::lock_guard<std::mutex> lock(messageMutex);
    Message msg;
    msg.from = from;
    msg.to = to;
    msg.content = content;
    userMessages[to].push_back(msg);
    // 清理该用户的过期消息
    cleanExpiredMessagesForUser(to);
    std::cout << "Message sent: " << from << " -> " << to << ": " << content << std::endl;
}

std::vector<Message> MessageManager::getMessagesForUser(const std::string& username) {
    std::lock_guard<std::mutex> lock(messageMutex);
    cleanExpiredMessagesForUser(username);
    return userMessages[username];
}

void MessageManager::cleanExpiredMessages() {
    time_t now = time(nullptr);
    for (auto& [user, messages] : userMessages) {
        auto it = messages.begin();
        while (it != messages.end()) {
            if (now - it->timestamp > 24 * 3600) {
                it = messages.erase(it);
            } else {
                ++it;
            }
        }
    }
}

void MessageManager::cleanExpiredMessagesForUser(const std::string& username) {
    time_t now = time(nullptr);
    auto& messages = userMessages[username];
    auto it = messages.begin();
    while (it != messages.end()) {
        if (now - it->timestamp > 24 * 3600) {
            it = messages.erase(it);
        } else {
            ++it;
        }
    }
}

// 全局消息管理器实例
MessageManager g_messageManager;