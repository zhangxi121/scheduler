#ifndef SEQUENCE_ID_H
#define SEQUENCE_ID_H

#include <atomic>
#include <mutex>
#include <vector>

namespace nmsp
{
#define DEFAULT_CONN_NUM 3000 

    class SequenceID
    {

    public:
        static void CreateInstance(uint32_t base, uint32_t capacity = DEFAULT_CONN_NUM);
        static SequenceID *GetInstance() { return m_instance; }
        ~SequenceID();

    private:
        SequenceID(uint32_t base, uint32_t capacity = DEFAULT_CONN_NUM);
        SequenceID(const SequenceID &) = delete;
        SequenceID(SequenceID &&) = delete;
        void operator=(const SequenceID &) = delete;

    public:
        uint64_t GenSequenceID(void);
        bool ReleaseSequenceID(uint64_t respID);

    private:
        enum State
        {
            STATE_UNUSE, // 重未使用过,
            STATE_USING, // 正在被使用,
            STATE_DEL    // 元素被删除的桶,
        };

        struct Arr
        {
            Arr() : pArr_(nullptr), cap_(0), beginNum_(0), idxCursor_(0), full_(false), loopNum_(0) {}
            Arr(const Arr &) = delete;
            Arr(Arr &&) = delete;
            void operator=(const Arr &) = delete;
            ~Arr()
            {
                if (pArr_)
                {
                    delete[] pArr_;
                }
                pArr_ = nullptr;
            }
            void init(uint32_t cap, uint64_t beginNum)
            {
                pArr_ = new State[cap]();
                cap_ = cap;
                beginNum_ = beginNum;
            }
            bool resetState(uint32_t idx)
            {
                if (pArr_[idx] == STATE_USING)
                {
                    pArr_[idx] = STATE_DEL;
                    return true;
                }
                return false;
            }

            State *pArr_;
            uint32_t cap_;
            uint64_t beginNum_;
            uint32_t idxCursor_;
            std::mutex mtx_;
            bool full_;
            uint8_t loopNum_;
        };

    private:
        uint32_t m_baseID;
        static const uint32_t ARRAY_NUMS = 10; // 数组的个数, 有32个数组 (32个vector变量为了使 iter++ 遍历快些),
        Arr **m_array;
        std::atomic_uint m_arrayIdx;

        static std::mutex m_mtx;
        static SequenceID *m_instance;

    private:
    };
}
#endif