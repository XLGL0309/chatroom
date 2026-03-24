#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include "network.h"

// 任务结构体
struct Task {
    SOCKET socket;
};

// 线程池类
class ThreadPool {
private:
    int m_numThreads;
    std::vector<std::thread> m_workers;
    std::atomic<bool> m_running;
    
    // 核心：任务队列
    std::queue<Task> m_taskQueue;
    std::mutex m_queueMutex;
    std::condition_variable m_cv;
    
    // 私有构造函数，防止外部实例化
    ThreadPool(int numThreads = 10);
    ~ThreadPool();
    
    // 禁止复制和赋值
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    
    void workerLoop();

public:
    void start();
    void stop();
    void addTask(SOCKET socket);
    
    // 静态方法获取单例实例
    static ThreadPool& getInstance(int numThreads = 10);
};

// 全局变量，存储最大线程数
extern const int MAX_THREADS;

#endif // THREADPOOL_H