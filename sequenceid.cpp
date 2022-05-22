#include "sequenceid.h"

using namespace std;

namespace nmsp
{
    std::mutex SequenceID::m_mtx;
    SequenceID *SequenceID::m_instance = nullptr;

    void SequenceID ::CreateInstance(uint32_t base, uint32_t capacity)
    {
        if (nullptr == m_instance)
        {
            unique_lock<std::mutex> mtx(SequenceID::m_mtx);
            if (nullptr == m_instance)
            {
                m_instance = new SequenceID(base, capacity);
            }
        }
    }

    SequenceID::SequenceID(uint32_t base, uint32_t capacity) : m_baseID(base), m_arrayIdx(0), m_array(nullptr)
    {
        int cap = capacity / ARRAY_NUMS;
        m_array = new Arr *[ARRAY_NUMS];

        for (int i = 0; i < ARRAY_NUMS; ++i)
        {
            m_array[i] = new Arr[cap]();
            m_array[i]->init(cap, m_baseID + i * cap);
        }
    }

    SequenceID::~SequenceID(void)
    {
        if (m_array)
        {
            delete[] m_array;
        }
        m_array = nullptr;
    }

    uint64_t SequenceID::GenSequenceID(void)
    {
        uint arrayIdx = m_arrayIdx;
        m_arrayIdx = (m_arrayIdx + 1) % ARRAY_NUMS;
        Arr *arr = m_array[arrayIdx];

        {
            std::lock_guard<std::mutex> lk(arr->mtx_);
            for (uint32_t i = arr->idxCursor_; i < arr->cap_; ++i)
            {
                if (i == arr->cap_ - 1)
                {
                    i = 0;
                    arr->loopNum_++;
                    if (arr->loopNum_ >= 2)
                    {
                        arr->full_ = true;
                        break; // 让走到 again_, 从 m_array[arrayIdx+1] 里面去申请,
                    }
                }
                if (arr->pArr_[i] != STATE_USING)
                {
                    arr->pArr_[i] = STATE_USING;
                    return arr->beginNum_ + i;
                }
                arr->idxCursor_++;
            }
        }

    again_:
        return GenSequenceID();
    }

    bool SequenceID::ReleaseSequenceID(uint64_t respID)
    {
        uint32_t cap = m_array[0]->cap_;
        uint32_t arrayIdx = (respID - m_baseID) / cap;
        uint32_t arrIdxCursor_ = (respID - m_baseID) % cap;
        return m_array[arrayIdx]->resetState(arrIdxCursor_);

        // if (arrIdxCursor_ + m_array[arrayIdx]->beginNum_ == respID)
        // {
        //     return m_array[arrayIdx]->resetState(arrIdxCursor_);
        // } return false; 
    }
}