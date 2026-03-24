/*
 * message.h
 * 消息管理功能的头文件
 * 功能：定义消息结构体和消息管理器类
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <vector>
#include <mutex>
#include <map>

/**
 * 消息结构体
 * 功能：存储消息信息
 */
struct Message {
    std::string from;    // 发送者
    std::string to;      // 接收者
    std::string content; // 消息内容
    time_t timestamp;    // 时间戳，用于清理过期消息

    /**
     * 构造函数
     * 功能：初始化消息结构体
     */
    Message();
};

/**
 * 消息管理器类
 * 功能：使用单例模式管理消息
 * 说明：采用懒汉式单例模式，确保线程安全
 */
class MessageManager {
private:
    /**
     * 私有构造函数
     * 功能：初始化消息管理器
     */
    MessageManager();
    
    /**
     * 析构函数
     */
    ~MessageManager();
    
    // 禁止复制和赋值
    MessageManager(const MessageManager&) = delete;
    MessageManager& operator=(const MessageManager&) = delete;

public:
    /**
     * 添加消息
     * 功能：添加一条新消息
     * 参数：from - 发送者
     *       to - 接收者
     *       content - 消息内容
     */
    void addMessage(const std::string& from, const std::string& to, const std::string& content);
    
    /**
     * 获取用户的消息
     * 功能：获取指定用户的所有消息
     * 参数：username - 用户名
     * 返回值：消息列表
     */
    std::vector<Message> getMessagesForUser(const std::string& username);
    
    /**
     * 清理过期消息
     * 功能：清理所有过期的消息
     */
    void cleanExpiredMessages();
    
    /**
     * 清理用户的过期消息
     * 功能：清理指定用户的过期消息
     * 参数：username - 用户名
     */
    void cleanExpiredMessagesForUser(const std::string& username);
    
    /**
     * 获取单例实例
     * 功能：获取消息管理器的单例实例
     * 返回值：MessageManager的引用
     */
    static MessageManager& getInstance();
};

#endif // MESSAGE_H