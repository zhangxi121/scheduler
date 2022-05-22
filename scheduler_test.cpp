
#include <chrono>
#include <iostream>

#include "scheduler.h"
#include "sequenceid.h"

using namespace std;
using namespace nmsp;

#define USE_THREAD_POOL 

void printANum(int a)
{
    std::cout << a << std::endl;
}

void printAString(std::string str)
{
    std::cout << str << std::endl;
}

int test_weekday()
{
#ifdef USE_THREAD_POOL
    ThreadPool *pool = new ThreadPool(3, 10);
    TimerEvent timerevent(pool); // 不会被阻塞, 正常执行 term 定时任务,
#else
    TimerEvent timerevent(nullptr);
#endif
    SequenceID::CreateInstance(2001);
    SequenceID *seqId = SequenceID::GetInstance();

    auto f = std::bind(printANum, 100);
    auto f2 = std::bind(printAString, "hahahahhaha");

    Scheduler scheduler(&timerevent, seqId);
    scheduler.AddTermTask("43 15 * * * 1,2,3,4,5, * * * *", f);
    scheduler.AddTermTask("43 15 * * * 1,2,3,4,5, * * * *", f2);
    scheduler.AddTermTask("44 15 * * * 1,2,3,4,5, * * * *", f);
    scheduler.AddTermTask("44 15 * * * 1,2,3,4,5, * * * *", f2);
    scheduler.AddTermTask("45 15 * * * 1,2,3,4,5, * * * *", f2);


    scheduler.StartTerm();

    // 给足够的时间让任务去执行, 否则任务还没执行, 程序就退出了, 保证 timerevent线程 和 threadpool 线程执行,
    std::this_thread::sleep_for(std::chrono::seconds(1000));

    return 0;
}

int test_monthday()
{
#ifdef USE_THREAD_POOL
    ThreadPool *pool = new ThreadPool(3, 10);
    TimerEvent timerevent(pool); // 不会被阻塞, 正常执行 term 定时任务,
#else
    TimerEvent timerevent(nullptr);
#endif
    SequenceID::CreateInstance(2001);
    SequenceID *seqId = SequenceID::GetInstance();

    auto f = std::bind(printANum, 100);
    auto f2 = std::bind(printAString, "hahahahhaha");

    Scheduler scheduler(&timerevent, seqId);
    scheduler.AddTermTask("42 15 28,4 * * * * * * *", f);
    scheduler.AddTermTask("42 15 28,4 * * * * * * *", f2);
    scheduler.AddTermTask("43 15 28,4 * * * * * * *", f2);
    scheduler.AddTermTask("44 15 28,4 * * * * * * *", f2);

    scheduler.StartTerm();

    // 给足够的时间让任务去执行, 否则任务还没执行, 程序就退出了, 保证 timerevent线程 和 threadpool 线程执行,
    std::this_thread::sleep_for(std::chrono::seconds(600)); 

    return 0;
}

int main01(int argc, char const *argv[])
{
    test_weekday();
    return 0;
}

int main(int argc, char const *argv[])
{
    test_monthday();
    return 0;
}
