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
public:
    ThreadPool(int numThreads = 10);
    ~ThreadPool();
    
    void start();
    void stop();
    void addTask(SOCKET socket);
    
private:
    int m_numThreads;
    std::vector<std::thread> m_workers;
    std::atomic<bool> m_running;
    
    // 核心：任务队列
    std::queue<Task> m_taskQueue;
    std::mutex m_queueMutex;
    std::condition_variable m_cv;
    
    void workerLoop();
};

// 全局变量，存储最大线程数
extern const int MAX_THREADS;

// 全局线程池实例
extern ThreadPool g_threadPool;

#endif // THREADPOOL_H