#ifndef SAFE_MAP_H
#define SAFE_MAP_H

#include <map>
#include <mutex>

template <typename Key, typename Val>
class SafeMap
{
public:
    typedef typename std::map<Key, Val>::iterator this_iterator;
    typedef typename std::map<Key, Val>::const_iterator this_const_iterator;
    Val &operator[](const Key &key)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        return m_dataMap[key];
    }
    int erase(const Key &key)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        return m_dataMap.erase(key);
    }

    this_iterator find(const Key &key)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        return m_dataMap.find(key);
    }
    this_const_iterator find(const Key &key) const
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        return m_dataMap.find(key);
    }

    this_iterator end()
    {
        return m_dataMap.end();
    }

    this_const_iterator end() const
    {
        return m_dataMap.end();
    }

private:
    std::map<Key, Val> m_dataMap;
    std::mutex m_mtx;
};

#endif // SAFE_MAP_H
