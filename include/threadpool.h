/*
 * threadpool.h
 * 线程池功能的头文件
 * 功能：定义线程池类，用于管理线程和处理任务
 */

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include "network.h"

/**
 * 任务结构体
 * 功能：存储线程池任务信息
 */
struct Task {
    SOCKET socket; // 客户端Socket
};

/**
 * 线程池类
 * 功能：使用单例模式管理线程池
 * 说明：采用懒汉式单例模式，确保线程安全
 */
class ThreadPool {
private:
    // 线程数量
    int m_numThreads;
    // 工作线程数组
    std::vector<std::thread> m_workers;
    // 线程池运行状态
    std::atomic<bool> m_running;
    
    // 核心：任务队列
    std::queue<Task> m_taskQueue;
    // 任务队列的互斥锁
    std::mutex m_queueMutex;
    // 条件变量，用于线程同步
    std::condition_variable m_cv;
    
    /**
     * 私有构造函数
     * 功能：初始化线程池
     * 参数：numThreads - 线程数量，默认为10
     */
    ThreadPool(int numThreads = 10);
    
    /**
     * 析构函数
     * 功能：停止线程池并清理资源
     */
    ~ThreadPool();
    
    // 禁止复制和赋值
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    
    /**
     * 工作线程循环函数
     * 功能：线程的主要工作逻辑，从任务队列获取任务并执行
     */
    void workerLoop();

public:
    /**
     * 启动线程池
     * 功能：创建并启动工作线程
     */
    void start();
    
    /**
     * 停止线程池
     * 功能：停止所有工作线程
     */
    void stop();
    
    /**
     * 添加任务到线程池
     * 功能：将客户端Socket添加到任务队列
     * 参数：socket - 客户端Socket
     */
    void addTask(SOCKET socket);
    
    /**
     * 获取单例实例
     * 功能：获取线程池的单例实例
     * 参数：numThreads - 线程数量，默认为10
     * 返回值：ThreadPool的引用
     */
    static ThreadPool& getInstance(int numThreads = 10);
};

// 全局变量，存储最大线程数
extern const int MAX_THREADS;

#endif // THREADPOOL_H