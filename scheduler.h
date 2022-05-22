#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <list>
#include <mutex>
#include <thread>
#include <tuple>
#include <vector>

#include "semaphore.h"
#include "sequenceid.h"
#include "timer_event.h"

// -----------------------------------
// crontab 示例,
// 第1列分钟1～59
// 第2列小时1～23（0表示子夜）
// 第3列日1～31
// 第4列月1～12
// 第5列星期0～6（0表示星期天）
// 第6列要运行的命令
// 1.
//   59 23 * * * /home/oracle/scripts/alert_log_archive.sh >/dev/null 2>&1
//   表示每天23点59分执行脚本/home/oracle/scripts/alert_log_archive.sh
// 2.
//   */5 * * * * /home/oracle/scripts/monitoring_alert_log.sh >/dev/null 2>&1
//   表示每5分钟执行一次脚本/home/oracle/scripts/monitoring_alert_log.sh
// 3.
//   0 20 * * 1-5 mail -s "**********" kerry@domain.name < /tmp/maildata
//   周一到周五每天下午 20:00 寄一封信给 kerry@domain.name
//
// -----------------------------------
//
// mgr_scheduler 定时任务示例,
// 1. 同样的, * 表述无效参数,
//   min  hour  day_of_month  month   day_of_year  day_of_week  week_of_month   week_day_of_month    mon_day_of_year   week_day_of_year
//   05    23     12            *         *           *            *               *                   *                   *
//   每月12号的的 23:05 做任务,
// 2.
//   min  hour  day_of_month  month   day_of_year  day_of_week  week_of_month   week_day_of_month    mon_day_of_year   week_day_of_year
//   05    23      *           1-9        *          1,3,5         *               *                   *                   *
//   1-9月的  Monday Wednesday Friday 的 23:05 做任务,
// 3.
//   min  hour  day_of_month  month   day_of_year  day_of_week  week_of_month   week_day_of_month    mon_day_of_year   week_day_of_year
//   05    23      *            *         *           *            *               3:1,2:3             *                   *
//   每个月第3周的的Monday  和第2周的Wednesday 执行任务,
// 4.
//   min  hour  day_of_month  month   day_of_year  day_of_week  week_of_month   week_day_of_month    mon_day_of_year   week_day_of_year
//   05    23      1,-1         *         *           *            *               *                   *                   *
//   每个月第一天或者最后一天 执行任务,
// 5.
//   min  hour  day_of_month  month   day_of_year  day_of_week  week_of_month   week_day_of_month    mon_day_of_year   week_day_of_year
//   05    23      *            *         *           *            *               *                   3:1,5:1             *
//   每年的 3.1 或者 5.1  执行任务,
// 6.
//   min  hour  day_of_month  month   day_of_year  day_of_week  week_of_month   week_day_of_month    mon_day_of_year   week_day_of_year
//   05    23      *            *         *           *            *               *                   *                  1:1,-1:7
//   每年的 第一周的第一天 和 最后一周的的第7天 执行任务,
//
// ...
// 目前暂时完成每周每天的任务,
// -----------------------------------

namespace nmsp
{
    class Scheduler
    {
    public:
        enum SchedulerType
        {
            UNKNOWN = 0,
            DAY_OF_WEEK = 1,
            DAY_OF_MONTH, // 包含每月的第一天或者最后一天, FIRST_LAST_DAY_OF_MONTH,
            WEEK_DAY_OF_MONTH,
            MONTH_DAY_OF_YEAR,
            WEEK_DAY_OF_YEAR
        };

        typedef std::tuple<Scheduler::SchedulerType, std::string, TimerCallback, uint64_t> TermInfo;

    public:
        Scheduler(TimerEvent *timerEvent, SequenceID *seqId);
        Scheduler(const Scheduler &) = delete;
        Scheduler(Scheduler &&) = delete;
        ~Scheduler() = default;

    public:
        //   min  hour  day_of_month  month   day_of_year  day_of_week  week_of_month   week_day_of_month    mon_day_of_year   week_day_of_year
        //   05    23      *            *         *           1,2,3            *               *                   *               1:1,-1:7
        //   day_of_week(1,2,3) 与 week_day_of_year(1:1,-1:7) 冲突就会加入失败, return false;
        bool AddTermTask(const char *fmt, TimerCallback cb, const char * = "* * * * * * * * * *");

        void StartTerm(void);

        std::list<std::string> DumpJob(void);

    private:
        bool CheckFmt(const char *fmt, TimerCallback cb);

        void RunJobsThread(void);

        void WaitForNextJob(void);

        void RecordSequence(uint64_t sequenceId);

        void ReleaseSequence(void);

    private:
        void Scheduler_DayOfWeeK(const TermInfo &term, TimerEvent *timerEvent, SequenceID *seqId);
        void Scheduler_DayOfMonth(const TermInfo &term, TimerEvent *timerEvent, SequenceID *seqId); // 包含每月的第一天或者最后一天, FIRST_LAST_DAY_OF_MONTH,
        void Scheduler_WeekDayOfMonth(const TermInfo &term, TimerEvent *timerEvent, SequenceID *seqId);
        void Scheduler_MonthDayOfYear(const TermInfo &term, TimerEvent *timerEvent, SequenceID *seqId);
        void Scheduler_WeekDayOfYear(const TermInfo &term, TimerEvent *timerEvent, SequenceID *seqId);

    public:
        // 与下一个整点的间隔时间, 返回秒钟,
        static time_t GetNextClockTimestamp();

        // 计算每个月的第一周的周一是日期是多少,
        static time_t GetMonthWeekDayTimestamp(uint32_t year, uint8_t month, int week, int weekday);

        // 计算每年的第一周的周一是日期是多少,
        static time_t GetYearWeekDayTimestamp(uint32_t year, int week, int weekday);

    private:
        TimerEvent *m_pTimerEvent;
        SequenceID *m_pSeqId;
        std::list<std::tuple<SchedulerType, std::string, TimerCallback, uint64_t>> m_termList; // uint64_6  expire = 0;
        std::mutex m_mtx;
        sem_t m_sem_thread_start;
        static const unsigned int m_sem_thread_start_value = 0;
        std::vector<uint64_t> m_vec_releaseSeq;
    };

}

#endif