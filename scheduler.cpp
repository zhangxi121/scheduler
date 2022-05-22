#include <assert.h>
#include <thread>
#include <time.h>

#include "scheduler.h"
#include "string_utils.h"

// 设置下一个小时的的任务,
// 如果在首次启动时候, 定时任务距离 time_now 只有10s, 可以有偏差, 这个任务可以漏掉, 首次启动 10s 以内的任务可以漏掉
//

namespace nmsp
{

//----------------------------------------
#define FIRST_LAST_DAY_OF_MONTH_SCHEDULER()                                                         \
    do                                                                                              \
    {                                                                                               \
        int term_sec = atoi(term_min.c_str()) * 60 + atoi(term_hour.c_str()) * 3600;                \
        int next_clock_sec_count = n_sec + n_min * 60 + n_hour * 3600;                              \
        int next_clock_delta_sec = next_clock_sec_count - term_sec;                                 \
                                                                                                    \
        if (0 < next_clock_delta_sec && next_clock_delta_sec <= 3600)                               \
        {                                                                                           \
            time_t tnow = time(nullptr);                                                            \
            struct tm *nowtm = localtime(&tnow);                                                    \
                                                                                                    \
            int now_delta_sec = atoi(term_min.c_str()) * 60 - (nowtm->tm_sec + nowtm->tm_min * 60); \
                                                                                                    \
            uint32_t sequence_id = seqId->GenSequenceID();                                          \
                                                                                                    \
            std::cout << "xxxxxxxx" << n_mday << "  " << item << " " << now_delta_sec << "sec"      \
                      << " " << sequence_id << " " << std::endl;                                    \
                                                                                                    \
            timerEvent->setTimeEvent(sequence_id, now_delta_sec * 1000, cb, false);                 \
            RecordSequence(sequence_id);                                                            \
            continue;                                                                               \
        }                                                                                           \
    } while (0);

    //----------------------------------------

    //----------------------------------------

    // using TermInfo = std::tuple<Scheduler::SchedulerType, std::string, TimerCallback, uint64_t> ;

    static bool IsLeapYear(int year)
    {
        return (year % 100 != 0 && year % 4 == 0) || (year % 400 == 0);
    }

    Scheduler::Scheduler(TimerEvent *timerEvent, SequenceID *seqId)
        : m_pTimerEvent(timerEvent), m_pSeqId(seqId)
    {
        sem_init(&m_sem_thread_start, 0, m_sem_thread_start_value);
        std::thread(&Scheduler::RunJobsThread, this).detach();
    }

    bool Scheduler::AddTermTask(const char *fmt, TimerCallback cb, const char *)
    {
        return CheckFmt(fmt, cb);
    }

    void Scheduler::StartTerm(void)
    {
        sem_post(&m_sem_thread_start);
    }

    std::list<std::string> Scheduler::DumpJob(void)
    {
        // 打印未来1个小时之内的job,
        std::list<std::string> li;
        return li;
    }

    bool Scheduler::CheckFmt(const char *fmt, TimerCallback cb)
    {
        std::vector<std::string> vec_timeinfo = SplitString(fmt, " ");

        // assert(vec_timeinfo.size() == 10);
        if (vec_timeinfo.size() != 10)
            return false;

        std::string min = vec_timeinfo[0];
        std::string hour = vec_timeinfo[1];
        std::string day_of_month = vec_timeinfo[2];
        std::string month = vec_timeinfo[3];
        std::string day_of_year = vec_timeinfo[4];
        std::string day_of_week = vec_timeinfo[5];
        std::string week_of_month = vec_timeinfo[6];
        std::string week_day_of_month = vec_timeinfo[7];
        std::string mon_day_of_year = vec_timeinfo[8];
        std::string week_day_of_year = vec_timeinfo[9];

        if (day_of_week != "*")
        {
            // SchedulerType::DAY_OF_WEEK ..
            if (day_of_month == "*" && month == "*" && day_of_year == "*" && week_of_month == "*" &&
                week_day_of_month == "*" && mon_day_of_year == "*" && week_day_of_year == "*" &&
                min != "*" && hour != "*")
            {
                SchedulerType type = DAY_OF_WEEK;
                std::string termStr = min + '|' + hour + '|' + day_of_week;
                std::lock_guard<std::mutex> lock(m_mtx);
                m_termList.push_back(std::make_tuple(type, termStr, cb, 0));
                return true;
            }
            return false;
        }

        if (day_of_month != "*")
        {
            if (month == "*" && day_of_year == "*" && day_of_week == "*" && week_of_month == "*" &&
                week_day_of_month == "*" && mon_day_of_year == "*" && week_day_of_year == "*" &&
                min != "*" && hour != "*")
            {
                SchedulerType type = DAY_OF_MONTH; // 包含每月的第一天或者最后一天, FIRST_LAST_DAY_OF_MONTH, -1表示最后一天,
                std::string termStr = min + '|' + hour + '|' + day_of_month;
                std::lock_guard<std::mutex> lock(m_mtx);
                m_termList.push_back(std::make_tuple(type, termStr, cb, 0));
                return true;
            }
            return false;
        }

        if (week_day_of_month != "*")
        {
            // 每月第几周的周几,比如如果executeDay的值是3:1,2:3表示每月第三周的周一和每月第二周的周三执行,
            if (day_of_month == "*" && month == "*" && day_of_year == "*" && day_of_week == "*" &&
                week_of_month == "*" && mon_day_of_year == "*" && week_day_of_year == "*" &&
                min != "*" && hour != "*")
            {
                SchedulerType type = WEEK_DAY_OF_MONTH;
                std::string termStr = min + '|' + hour + '|' + week_day_of_month;
                std::lock_guard<std::mutex> lock(m_mtx);
                m_termList.push_back(std::make_tuple(type, termStr, cb, 0));
                return true;
            }
            return false;
        }

        if (mon_day_of_year != "*")
        {
            // 每年几月几号，例如如果executeDay的值是3:1表示3月1号,5:1表示5月1号,
            if (day_of_month == "*" && month == "*" && day_of_year == "*" && day_of_week == "*" &&
                week_of_month == "*" && week_day_of_month == "*" && week_day_of_year == "*" &&
                min != "*" && hour != "*")
            {
                SchedulerType type = MONTH_DAY_OF_YEAR;
                std::string termStr = min + '|' + hour + '|' + mon_day_of_year;
                std::lock_guard<std::mutex> lock(m_mtx);
                m_termList.push_back(std::make_tuple(type, termStr, cb, 0));
                return true;
            }
            return false;
        }

        if (week_day_of_year != "*")
        {
            // 每年第一个或者最后一个周几，比如如果executeDay的值是1:1表示第一个周一,0:7表示最后一个周日，冒号前1表示第一个，0表示最后一个；冒号后的表示具体周几,
            if (day_of_month == "*" && month == "*" && day_of_year == "*" && day_of_week == "*" &&
                week_of_month == "*" && week_day_of_month == "*" && mon_day_of_year == "*" &&
                min != "*" && hour != "*")
            {
                SchedulerType type = WEEK_DAY_OF_YEAR;
                std::string termStr = min + '|' + hour + '|' + week_day_of_year;
                std::lock_guard<std::mutex> lock(m_mtx);
                m_termList.push_back(std::make_tuple(type, termStr, cb, 0));
                return true;
            }
            return false;
        }
        return false;
    }

    void Scheduler::RunJobsThread(void)
    {
        sem_wait(&m_sem_thread_start);
        int sleep_sec = 3600; // 休眠3600s,

        // 一定要 StartTerm() 之后,本线程再运行, 这样 timerevent(threadpool) 中的定时任务是正常的,
        // 否则 timerevent(threadpool) 的 threadpool 会出现逻辑上的混乱,

        while (1)
        {
            ReleaseSequence();
            WaitForNextJob();

            // 每次等 WaitForNextJob() 执行完以后计算当前时间,

            time_t t = time(nullptr);
            struct tm *nowtm = localtime(&t);
            int tm_sec = nowtm->tm_sec;
            int tm_min = nowtm->tm_min;

            if (tm_sec != 0 && tm_min != 0)
            {
                sleep_sec = 3600 - (tm_sec + tm_min * 60);
            }

            std::this_thread::sleep_for(std::chrono::seconds(sleep_sec));
        }
        return;
    }

    void Scheduler::WaitForNextJob(void)
    {
        for (std::tuple<SchedulerType, std::string, TimerCallback, uint64_t> &item : m_termList)
        {
            std::cout << m_termList.size() << std::endl;
            switch (std::get<0>(item))
            {
            case SchedulerType::DAY_OF_WEEK:
                this->Scheduler_DayOfWeeK(item, m_pTimerEvent, m_pSeqId);
                break;
            case SchedulerType::DAY_OF_MONTH:
                this->Scheduler_DayOfMonth(item, m_pTimerEvent, m_pSeqId);
                break;
            case SchedulerType::WEEK_DAY_OF_MONTH:
                this->Scheduler_WeekDayOfMonth(item, m_pTimerEvent, m_pSeqId);
                break;
            case SchedulerType::MONTH_DAY_OF_YEAR:
                this->Scheduler_MonthDayOfYear(item, m_pTimerEvent, m_pSeqId);
                break;
            case SchedulerType::WEEK_DAY_OF_YEAR:
                this->Scheduler_WeekDayOfYear(item, m_pTimerEvent, m_pSeqId);
                break;
            default:
                break;
            }
        }
    }

    void Scheduler::RecordSequence(uint64_t sequenceId)
    {
        m_vec_releaseSeq.emplace_back(sequenceId);
    }

    void Scheduler::ReleaseSequence(void)
    {
        for (uint64_t &item : m_vec_releaseSeq)
        {
            m_pSeqId->ReleaseSequenceID(item);
        }
        m_vec_releaseSeq.clear();
    }

    //----------------

    time_t Scheduler::GetNextClockTimestamp()
    {
        int durSec = 0;
        time_t t = time(nullptr);
        struct tm *nowtm = localtime(&t);
        int tm_sec = nowtm->tm_sec;
        int tm_min = nowtm->tm_min;

        if (tm_sec == 0 && tm_min == 0)
        {
            durSec = 3600;
        }
        else
        {
            durSec = 3600 - (tm_sec + tm_min * 60);
        }
        return (t + durSec);
    }

    time_t Scheduler::GetMonthWeekDayTimestamp(uint32_t year, uint8_t month, int week, int weekday)
    {
        struct tm that_tm;
        memset(&that_tm, 0, sizeof(struct tm));
        // 这个月的第一天是从周五开始的, 1:2 怎么计算???
        //
        that_tm.tm_mday = 1;
        that_tm.tm_mon = month;
        that_tm.tm_year = year - 1900;

        time_t monthFirstDay = mktime(&that_tm);

        // struct tm *ptm = localtime(&monthFirstDay);
        // ptm->tm_wday;

        return monthFirstDay;
    }

    time_t Scheduler::GetYearWeekDayTimestamp(uint32_t year, int week, int weekday)
    {
        struct tm that_tm;
        memset(&that_tm, 0, sizeof(struct tm));

        // 价格今年的第一天是周五, 1:2 怎么计算??
        //
        that_tm.tm_yday = week * 7 + weekday;
        that_tm.tm_year = year - 1900;

        time_t monthFirstDay = mktime(&that_tm);

        // struct tm *ptm = localtime(&monthFirstDay);

        return monthFirstDay;
    }

    void Scheduler::Scheduler_DayOfWeeK(const TermInfo &term, TimerEvent *timerEvent, SequenceID *seqId)
    {

        std::string termStr = std::get<1>(term);
        TimerCallback cb = std::get<2>(term);
        // uint64_t expire = std::get<3>(term);

        std::cout << termStr << std::endl;

        time_t next_clock = GetNextClockTimestamp();

        struct tm *next_tm = localtime(&next_clock);
        int n_sec = next_tm->tm_sec;
        int n_min = next_tm->tm_min;
        int n_hour = next_tm->tm_hour;
        int n_wday = next_tm->tm_wday; // 0-6, 周日是0,

        std::vector<std::string> splitTermStr = SplitString(termStr, "|");
        std::string term_min = splitTermStr[0];
        std::string term_hour = splitTermStr[1];
        std::string term_weekDay = splitTermStr[2];

        std::vector<std::string> vec_weekday = SplitString(term_weekDay, ",");
        for (std::string &item : vec_weekday)
        {
            if (atoi(item.c_str()) == n_wday)
            {
                int term_sec = atoi(term_min.c_str()) * 60 + atoi(term_hour.c_str()) * 3600;
                int next_clock_sec_count = n_sec + n_min * 60 + n_hour * 3600;
                int next_clock_delta_sec = next_clock_sec_count - term_sec;

                if (0 < next_clock_delta_sec && next_clock_delta_sec <= 3600)
                {
                    time_t tnow = time(nullptr);
                    struct tm *nowtm = localtime(&tnow);

                    int now_delta_sec = atoi(term_min.c_str()) * 60 - (nowtm->tm_sec + nowtm->tm_min * 60);

                    uint32_t sequence_id = seqId->GenSequenceID();

                    std::cout << "xxxxxxxx " << n_wday << "  " << item << " " << now_delta_sec << "sec"
                              << " " << sequence_id << " " << std::endl;

                    timerEvent->setTimeEvent(sequence_id, now_delta_sec * 1000, cb, false);
                    RecordSequence(sequence_id);
                    continue;
                }
            }
        }
    }

    void Scheduler::Scheduler_DayOfMonth(const TermInfo &term, TimerEvent *timerEvent, SequenceID *seqId)
    {
        // 包含每月的第一天或者最后一天, FIRST_LAST_DAY_OF_MONTH,

        std::string termStr = std::get<1>(term);
        TimerCallback cb = std::get<2>(term);
        // uint64_t expire = std::get<3>(term);

        time_t next_clock = GetNextClockTimestamp();

        struct tm *next_tm = localtime(&next_clock);
        int n_sec = next_tm->tm_sec;
        int n_min = next_tm->tm_min;
        int n_hour = next_tm->tm_hour;
        int n_mday = next_tm->tm_mday;        // 1-31,
        int n_month = next_tm->tm_mon;        // [0-11],
        int n_year = next_tm->tm_year + 1900; // Year-1900,

        std::vector<std::string> splitTermStr = SplitString(termStr, "|");
        std::string term_min = splitTermStr[0];
        std::string term_hour = splitTermStr[1];
        std::string term_monthDay = splitTermStr[2]; // 查看 term_monthDay 有没有 -1, 将 -1 调整为每个月的最后一天,

        std::vector<std::string> vec_monthday = SplitString(term_monthDay, ",");
        for (std::string &item : vec_monthday)
        {
            if (atoi(item.c_str()) == -1)
            {
                // 每个月的最后一天, 计算下一个小时的日期, 然后查看是不是当月的最后一天,

                bool bLeap = IsLeapYear(n_year);

                switch (n_month + 1)
                {
                case 1:
                case 3:
                case 5:
                case 7:
                case 8:
                case 10:
                case 12:
                    if (31 == n_mday)
                    {
                        FIRST_LAST_DAY_OF_MONTH_SCHEDULER();
                    }
                    break;
                case 2:
                    if (bLeap && 29 == n_mday)
                    {
                        FIRST_LAST_DAY_OF_MONTH_SCHEDULER();
                    }
                    else if (!bLeap && 28 == n_mday)
                    {
                        FIRST_LAST_DAY_OF_MONTH_SCHEDULER();
                    }
                    break;
                case 4:
                case 6:
                case 9:
                case 11:
                    if (30 == n_mday)
                    {
                        FIRST_LAST_DAY_OF_MONTH_SCHEDULER();
                    }
                    break;
                }
            }
            if (atoi(item.c_str()) == n_mday)
            {
                int term_sec = atoi(term_min.c_str()) * 60 + atoi(term_hour.c_str()) * 3600;
                int next_clock_sec_count = n_sec + n_min * 60 + n_hour * 3600;
                int next_clock_delta_sec = next_clock_sec_count - term_sec;

                if (0 < next_clock_delta_sec && next_clock_delta_sec <= 3600)
                {
                    time_t tnow = time(nullptr);
                    struct tm *nowtm = localtime(&tnow);

                    int now_delta_sec = atoi(term_min.c_str()) * 60 - (nowtm->tm_sec + nowtm->tm_min * 60);

                    uint32_t sequence_id = seqId->GenSequenceID();

                    std::cout << "xxxxxxxx" << n_mday << "  " << item << " " << now_delta_sec << "sec"
                              << " " << sequence_id << " " << std::endl;

                    timerEvent->setTimeEvent(sequence_id, now_delta_sec * 1000, cb, false);
                    RecordSequence(sequence_id);
                    continue;
                }
            }
        }
    }

    void Scheduler::Scheduler_WeekDayOfMonth(const TermInfo &term, TimerEvent *timerEvent, SequenceID *seqId)
    {
        // 每月第几周的周几,比如如果executeDay的值是3:1,2:3表示每月第三周的周一和每月第二周的周三执行

        std::string termStr = std::get<1>(term);
        TimerCallback cb = std::get<2>(term);
        // uint64_t expire = std::get<3>(term);

        std::vector<std::string> splitTermStr = SplitString(termStr, "|");
        std::string term_min = splitTermStr[0];
        std::string term_hour = splitTermStr[1];
        std::string term_monthWeekDay = splitTermStr[2];

        std::vector<std::pair<int, int>> vec_weekDay;

        std::vector<std::string> vec_monthWeekday = SplitString(term_monthWeekDay, ",");
        for (std::string &item : vec_monthWeekday)
        {
            std::vector<std::string> splitWeekDayStr = SplitString(item, ":");
            vec_weekDay.emplace_back(make_pair(atoi(splitWeekDayStr[0].c_str()), atoi(splitWeekDayStr[1].c_str())));
        }

        time_t next_clock = GetNextClockTimestamp();

        struct tm *next_tm = localtime(&next_clock);
        // int n_sec = next_tm->tm_sec;
        // int n_min = next_tm->tm_min;
        // int n_hour = next_tm->tm_hour;
        // int n_wday = next_tm->tm_wday;        // 0-6, 周日是0,
        // int n_mday = next_tm->tm_mday;        // 1-31,
        int n_month = next_tm->tm_mon;        // [0-11],
        int n_year = next_tm->tm_year + 1900; // Year-1900,

        for (const std::pair<int, int> &item : vec_weekDay)
        {
            int week = item.first;
            int day = item.second;

            GetMonthWeekDayTimestamp(n_year, n_month, week, day - 1);

            // 将这个转化为这个该月的几月几号来处理,,
            //
            //
        }
    }

    void Scheduler::Scheduler_MonthDayOfYear(const TermInfo &term, TimerEvent *timerEvent, SequenceID *seqId)
    {
        // 每年几月几号，例如如果executeDay的值是3:1表示3月1号,5:1表示5月1号,

        std::string termStr = std::get<1>(term);
        TimerCallback cb = std::get<2>(term);
        // uint64_t expire = std::get<3>(term);

        std::vector<std::string> splitTermStr = SplitString(termStr, "|");
        std::string term_min = splitTermStr[0];
        std::string term_hour = splitTermStr[1];
        std::string term_yearMonthDay = splitTermStr[2];

        std::vector<std::pair<int, int>> vec_monthDay;

        std::vector<std::string> vec_yearMonthDay = SplitString(term_yearMonthDay, ",");
        for (std::string &item : vec_yearMonthDay)
        {
            std::vector<std::string> splitMonthDayStr = SplitString(item, ":");
            vec_monthDay.emplace_back(make_pair(atoi(splitMonthDayStr[0].c_str()), atoi(splitMonthDayStr[1].c_str())));
        }

        time_t next_clock = GetNextClockTimestamp();

        struct tm *next_tm = localtime(&next_clock);
        int n_sec = next_tm->tm_sec;
        int n_min = next_tm->tm_min;
        int n_hour = next_tm->tm_hour;
        int n_mday = next_tm->tm_mday; // 1-31,
        int n_month = next_tm->tm_mon; // [0-11],
        // int n_year = next_tm->tm_year + 1900; // Year-1900,

        for (const std::pair<int, int> &item : vec_monthDay)
        {
            int month = item.first;
            int day = item.second;

            if (n_month + 1 == month && n_mday == day)
            {
                int term_sec = atoi(term_min.c_str()) * 60 + atoi(term_hour.c_str()) * 3600;
                int next_clock_sec_count = n_sec + n_min * 60 + n_hour * 3600;
                int next_clock_delta_sec = next_clock_sec_count - term_sec;

                if (0 < next_clock_delta_sec && next_clock_delta_sec <= 3600)
                {
                    time_t tnow = time(nullptr);
                    struct tm *nowtm = localtime(&tnow);

                    int now_delta_sec = atoi(term_min.c_str()) * 60 - (nowtm->tm_sec + nowtm->tm_min * 60);

                    uint32_t sequence_id = seqId->GenSequenceID();

                    std::cout << "xxxxxxxx" << month << "  " << day << " " << now_delta_sec << "sec"
                              << " " << sequence_id << " " << std::endl;

                    timerEvent->setTimeEvent(sequence_id, now_delta_sec * 1000, cb, false);
                    RecordSequence(sequence_id);
                    continue;
                }
                continue;
            }
        }
    }

    void Scheduler::Scheduler_WeekDayOfYear(const TermInfo &term, TimerEvent *timerEvent, SequenceID *seqId)
    {
        // 每年第一个或者最后一个周几，比如如果executeDay的值是1:1表示第一个周一,0:7表示最后一个周日，冒号前1表示第一个，0表示最后一个；冒号后的表示具体周几

        std::string termStr = std::get<1>(term);
        TimerCallback cb = std::get<2>(term);
        // uint64_t expire = std::get<3>(term);

        std::vector<std::string> splitTermStr = SplitString(termStr, "|");
        std::string term_min = splitTermStr[0];
        std::string term_hour = splitTermStr[1];
        std::string term_yearWeekDay = splitTermStr[2];

        std::vector<std::pair<int, int>> vec_weekDay;

        std::vector<std::string> vec_yearWeekDay = SplitString(term_yearWeekDay, ",");
        for (std::string &item : vec_yearWeekDay)
        {
            std::vector<std::string> splitWeekDayStr = SplitString(item, ":");
            vec_weekDay.emplace_back(make_pair(atoi(splitWeekDayStr[0].c_str()), atoi(splitWeekDayStr[1].c_str())));
        }

        time_t next_clock = GetNextClockTimestamp();

        struct tm *next_tm = localtime(&next_clock);
        // int n_sec = next_tm->tm_sec;
        // int n_min = next_tm->tm_min;
        // int n_hour = next_tm->tm_hour;
        // int n_wday = next_tm->tm_wday;        // 0-6, 周日是0,
        int n_year = next_tm->tm_year + 1900; // Year-1900,

        for (const std::pair<int, int> &item : vec_weekDay)
        {
            int week = item.first;
            int day = item.second;

            GetYearWeekDayTimestamp(n_year, week, day - 1);
        }
    }

}
