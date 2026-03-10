#include "../include/message.h"
#include <iostream>

Message::Message() : timestamp(time(nullptr)) {}

void MessageManager::addMessage(const std::string& from, const std::string& to, const std::string& content) {
    std::lock_guard<std::mutex> lock(messageMutex);
    Message msg;
    msg.from = from;
    msg.to = to;
    msg.content = content;
    messageList.push_back(msg);
    std::cout << "Message sent: " << from << " -> " << to << ": " << content << std::endl;
}

std::vector<Message> MessageManager::getMessagesForUser(const std::string& username) {
    std::lock_guard<std::mutex> lock(messageMutex);
    cleanExpiredMessages();
    
    std::vector<Message> userMessages;
    for (const auto& msg : messageList) {
        if (msg.to == username) {
            userMessages.push_back(msg);
        }
    }
    return userMessages;
}

void MessageManager::cleanExpiredMessages() {
    time_t now = time(nullptr);
    auto it = messageList.begin();
    while (it != messageList.end()) {
        if (now - it->timestamp > 24 * 3600) {
            it = messageList.erase(it);
        } else {
            ++it;
        }
    }
}

// 全局消息管理器实例
MessageManager g_messageManager;