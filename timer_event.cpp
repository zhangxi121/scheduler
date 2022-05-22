#include <iostream>
#include <unistd.h>

#include "common.h"
#include "timer_event.h"

TimerEvent::TimerEvent(ThreadPool *threadpool)
    : m_epollfd(::epoll_create1(EPOLL_CLOEXEC)),
      m_events(m_initEventListSize)
{
    ///<线程池，
    m_threadPoolPtr = threadpool;

    ///<创建一个线程做epool事件监听
    std::thread t(&TimerEvent::handleTimerfdInEpoll, this);
    t.detach();
}

TimerEvent::~TimerEvent()
{
    ::close(m_epollfd);
}

bool TimerEvent::setTimeEvent(const uint64_t timerId, const uint32_t ms, TimerCallback cb, const bool isPeriodic)
{
    if (timerIdIsExist(timerId))
    {
        return false;
    }
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0)
    {
        return false;
    }
    if (!timerFdSetTime(timerfd, ms, isPeriodic))
    {
        return false;
    }
    if (!epollAddTimerFd(timerId, timerfd))
    {
        return false;
    }
    m_timerMap[timerId].timerFunc = cb;
    m_timerMap[timerId].isPeriodic = isPeriodic;
    return true;
}
bool TimerEvent::cancelTimeEvent(const uint64_t timerId)
{
    if (!timerIdIsExist(timerId))
    {
        return false;
    }
    if (!stopTimerfdSetTime(m_timerMap[timerId].timerfd))
    {
        return false;
    }
    ///< 从epoll循环中去掉fd的监听
    epollDelTimerFd(m_timerMap[timerId].timerfd);
    ///< 从map中删除timerId
    m_timerMap.erase(timerId);
    return true;
}

bool TimerEvent::timerIdIsExist(const uint64_t timerId)
{
    return m_timerMap.find(timerId) != m_timerMap.end();
}

bool TimerEvent::timerFdSetTime(const int timerfd, const uint32_t ms, const bool isPeriodic)
{
    struct itimerspec newValue;
    memZero(&newValue, sizeof newValue);
    if (ms >= 1000)
    {
        newValue.it_value.tv_sec = ms / 1000;
    }
    newValue.it_value.tv_nsec = (ms % 1000) * 1000;
    if (isPeriodic)
    {
        newValue.it_interval = newValue.it_value;
    }
    if (::timerfd_settime(timerfd, 0, &newValue, NULL))
    {
        return false;
    }
    return true;
}

bool TimerEvent::stopTimerfdSetTime(const int timerfd)
{
    struct itimerspec newValue;
    memZero(&newValue, sizeof newValue);
    if (::timerfd_settime(timerfd, 0, &newValue, NULL))
    {
        return false;
    }
    return true;
}

bool TimerEvent::epollAddTimerFd(const uint64_t timerId, const int timerfd)
{
    struct epoll_event event;
    TimerInfo info;
    info.timerfd = timerfd;
    info.timerId = timerId;
    m_timerMap[timerId] = info;
    memZero(&event, sizeof event);
    event.data.ptr = &m_timerMap[timerId];
    event.events = EPOLLIN;
    if (::epoll_ctl(m_epollfd, EPOLL_CTL_ADD, timerfd, &event) < 0)
    {
        m_timerMap.erase(timerId);
        return false;
    }
    return true;
}

bool TimerEvent::epollDelTimerFd(const int timerfd)
{
    struct epoll_event event;
    memZero(&event, sizeof event);
    event.events = EPOLLOUT;
    event.data.fd = timerfd;
    if (::epoll_ctl(m_epollfd, EPOLL_CTL_DEL, timerfd, &event) < 0)
    {
        return false;
    }
    return true;
}

void TimerEvent::readTimerfd(int timerfd)
{
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    if (n != sizeof howmany)
    {
        ///< error log
        return;
    }
}

void TimerEvent::handleTimerfdInEpoll()
{
    while (true)
    {
        int numEvents = ::epoll_wait(m_epollfd,
                                     &*m_events.begin(),
                                     static_cast<int>(m_events.size()),
                                     1000);
        ///< 事件触发之后就将函数提交给线程池去执行
        for (int i = 0; i < numEvents; i++)
        {
            TimerInfo *infoPtr = static_cast<TimerInfo *>(m_events[i].data.ptr);
            readTimerfd(infoPtr->timerfd);
            if (!infoPtr->isPeriodic)
            {
                TimerCallback funcA = infoPtr->timerFunc; // 在销毁前先保存一下, 执行一遍该函数,
                cancelTimeEvent(infoPtr->timerId);
                std::thread(funcA).detach();  // timerevent 一次性任务不是很多, 多以这里另开线程去执行任务,
            }
            else
            {
                if (m_threadPoolPtr == nullptr)
                {
                    infoPtr->timerFunc(); //你要是么有线程池，就直接执行回调
                }
                else
                {
                    LOG("=======  m_threadPoolPtr->AddTask =======");
                    m_threadPoolPtr->AddTask(infoPtr->timerFunc); //推荐其他线程执行，保证定时准确
                }
            }
        }
        ///< 说明一次触发的事件太多，扩大容量
        if (static_cast<size_t>(numEvents) == m_events.size())
        {
            m_events.resize(m_events.size() * 2);
        }
    }
}
