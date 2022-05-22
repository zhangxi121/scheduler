#ifndef SAFE_QUEUE_H
#define SAFE_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class SafeQueue
{
public:
    SafeQueue() = default;
    SafeQueue(const SafeQueue &) = delete;
    SafeQueue(SafeQueue &&) = delete;
    SafeQueue &operator=(const SafeQueue &) = delete;
    ~SafeQueue() = default;

public:
    void Push(const T &data)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_queue.push(data);
        m_cond.notify_one();
    }

    T Pop()
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        while (m_queue.empty())
        {
            m_cond.wait(lock);
        }

        T data = m_queue.front();
        m_queue.pop();
        return data;
    }

    bool Empty()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_queue.empty();
    }

    std::size_t Size()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_queue.size();
    }

private:
    std::queue<T> m_queue;
    std::mutex m_mtx;
    std::condition_variable m_cond;
};

#endif // SAFE_QUEUE_H
