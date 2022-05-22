#ifndef CCTIMER_H
#define CCTIMER_H

#include <cstring>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>

#include "safemap.h"
#include "threadpool.h"

typedef std::function<void()> TimerCallback;

inline void memZero(void *p, size_t n)
{
    memset(p, 0, n);
}

struct TimerInfo
{
    int timerfd;
    TimerCallback timerFunc;
    uint64_t timerId;
    bool isPeriodic;
};

class TimerEvent
{
public:
    TimerEvent(ThreadPool *threadpool);
    ~TimerEvent();
    bool setTimeEvent(const uint64_t timerId, const uint32_t ms, TimerCallback cb, const bool isPeriodic = false);
    bool cancelTimeEvent(const uint64_t timerId);

private:
    void handleTimerfdInEpoll();
    bool timerIdIsExist(const uint64_t timerId);
    bool epollAddTimerFd(const uint64_t timerId, const int timerfd);
    bool epollDelTimerFd(const int timerfd);
    bool timerFdSetTime(const int timerfd, const uint32_t ms, const bool isPeriodic = false);
    bool stopTimerfdSetTime(const int timerfd);
    void readTimerfd(int timerfd);

private:
    static const int m_initEventListSize = 16;
    ThreadPool *m_threadPoolPtr;
    const int m_epollfd;
    typedef std::vector<struct epoll_event> EventList;
    ///<触发的事件填充
    EventList m_events;                      // 使用 m_initEventListSize 来初始化 EventList 个数,
    SafeMap<uint64_t, TimerInfo> m_timerMap; // 这个map如果没有多线程去操作同一个定时器类对象的话，可以换成普通的map
};

#endif // CCTIMER_H
