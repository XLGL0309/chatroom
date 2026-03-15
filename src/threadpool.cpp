#include "../include/threadpool.h"
#include <iostream>
#include <chrono>
#include <string>
#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#include <string.h>
#endif

ThreadPool::ThreadPool(int numThreads) : m_numThreads(numThreads), m_running(false) {
}

ThreadPool::~ThreadPool() {
    stop();
}

void ThreadPool::start() {
    m_running = true;
    for (int i = 0; i < m_numThreads; ++i) {
        m_workers.emplace_back(&ThreadPool::workerLoop, this);
    }
    std::cout << "ThreadPool started with " << m_numThreads << " threads" << std::endl;
}

void ThreadPool::stop() {
    m_running = false;
    m_cv.notify_all(); // 唤醒所有等待的线程
    
    for (auto& t : m_workers) {
        if (t.joinable()) t.join();
    }
    m_workers.clear();
    std::cout << "ThreadPool stopped" << std::endl;
}

// 主线程调用这里：只负责把任务放进队列，立刻返回
void ThreadPool::addTask(SOCKET socket, const std::string& ip) {
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_taskQueue.push({socket, ip});
    }
    m_cv.notify_one(); // 唤醒一个工作线程
}

// 工作线程在这里运行
void ThreadPool::workerLoop() {
    while (m_running) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            
            // 等待，直到队列不为空 或 停止运行
            m_cv.wait(lock, [this] {  
                return !m_taskQueue.empty() || !m_running;  
            });
            
            if (!m_running && m_taskQueue.empty()) return;
            
            // 取出任务
            task = m_taskQueue.front();
            m_taskQueue.pop();
        } // 锁在这里释放，处理任务时不持有锁
        
        // 处理任务（锁外执行）
        handleClientConnection(task.socket, task.ip);
    }
}

// 全局变量，存储最大线程数
const int MAX_THREADS = []() -> int {
    int hardwareThreads = std::thread::hardware_concurrency();
    int baseThreads = hardwareThreads > 0 ? hardwareThreads : 10;
    
    // IO 密集型优化：设置为 核心数 * 2
    // 例如：8核16线程的CPU，我们就开32个线程
    return baseThreads * 2;
}();

// 全局实例
ThreadPool g_threadPool(MAX_THREADS); // 使用硬件支持的最大线程数