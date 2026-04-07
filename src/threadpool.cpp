/*
 * threadpool.cpp
 * 线程池功能的实现文件
 * 功能：实现线程池的方法，用于管理线程和处理任务
 */

#include "threadpool.h"
#include "network.h"
#include <iostream>
#include <string>

/**
 * 构造函数
 * 功能：初始化线程池
 * 参数：numThreads - 线程数量
 */
ThreadPool::ThreadPool(int numThreads) : m_numThreads(numThreads), m_running(false)
{
}

/**
 * 析构函数
 * 功能：停止线程池并清理资源
 */
ThreadPool::~ThreadPool()
{
    stop();
}

/**
 * 启动线程池
 * 功能：创建并启动工作线程
 */
void ThreadPool::start()
{
    m_running = true;
    for (int i = 0; i < m_numThreads; ++i)
    {
        m_workers.emplace_back(&ThreadPool::workerLoop, this);
    }
    std::cout << "Thread pool started, thread count: " << m_numThreads << std::endl;
}

/**
 * 停止线程池
 * 功能：停止所有工作线程
 */
void ThreadPool::stop()
{
    m_running = false;
    m_cv.notify_all();
    
    for (auto& t : m_workers)
    {
        if (t.joinable())
            t.join();
    }
    m_workers.clear();
    std::cout << "Thread pool stopped" << std::endl;
}

/**
 * 添加任务到线程池
 * 功能：将客户端Socket添加到任务队列
 * 参数：socket - 客户端Socket
 */
void ThreadPool::addTask(SOCKET socket)
{
    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_taskQueue.push({socket});
    m_cv.notify_one();
}

/**
 * 工作线程循环函数
 * 功能：线程的主要工作逻辑，从任务队列获取任务并执行
 */
void ThreadPool::workerLoop()
{
    while (m_running)
    {
        Task task;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_cv.wait(lock, [this] { return !m_taskQueue.empty() || !m_running; });
            if (!m_running)
                return;
            task = m_taskQueue.front();
            m_taskQueue.pop();
        }
        handleClientConnection(task.socket);
    }
}

/**
 * 最大线程数常量
 * 功能：根据硬件线程数动态计算最大线程数
 */
const int MAX_THREADS = []() -> int
{
    int hardwareThreads = std::thread::hardware_concurrency();
    int baseThreads = hardwareThreads > 0 ? hardwareThreads : 10;
    return baseThreads * 2;
}();

/**
 * 获取单例实例
 * 功能：获取线程池的单例实例
 * 参数：numThreads - 线程数量
 * 返回值：ThreadPool的引用
 */
ThreadPool& ThreadPool::getInstance(int numThreads)
{
    static ThreadPool instance(numThreads);
    return instance;
}
