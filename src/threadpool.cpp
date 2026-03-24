#include "../include/threadpool.h"
#include "../include/network.h"
#include <iostream>
#include <string>

ThreadPool::ThreadPool(int numThreads) : m_numThreads(numThreads), m_running(false)
{
}

ThreadPool::~ThreadPool()
{
    stop();
}

void ThreadPool::start()
{
    m_running = true;
    for (int i = 0; i < m_numThreads; ++i)
    {
        m_workers.emplace_back(&ThreadPool::workerLoop, this);
    }
    std::cout << "ThreadPool started with " << m_numThreads << " threads" << std::endl;
}

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
    std::cout << "ThreadPool stopped" << std::endl;
}

void ThreadPool::addTask(SOCKET socket)
{
    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_taskQueue.push({socket});
    m_cv.notify_one();
}

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

const int MAX_THREADS = []() -> int
{
    int hardwareThreads = std::thread::hardware_concurrency();
    int baseThreads = hardwareThreads > 0 ? hardwareThreads : 10;
    return baseThreads * 2;
}();

ThreadPool& ThreadPool::getInstance(int numThreads)
{
    static ThreadPool instance(numThreads);
    return instance;
}
