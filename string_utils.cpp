#include "string_utils.h"

namespace nmsp
{
    std::string &replace_all(std::string &str, const std::string &old_value, const std::string &new_value)
    {
        while (true)
        {
            std::string::size_type pos(0);
            if ((pos = str.find(old_value)) != std::string::npos)
                str.replace(pos, old_value.length(), new_value);
            else
                break;
        }
        return str;
    }

    std::string &replace_all_distinct(std::string &str, const std::string &old_value, const std::string &new_value)
    {
        for (std::string::size_type pos(0); pos != std::string::npos; pos += new_value.length())
        {
            if ((pos = str.find(old_value, pos)) != std::string::npos)
                str.replace(pos, old_value.length(), new_value);
            else
                break;
        }
        return str;
    }

    std::vector<std::string> SplitString(const std::string &s, const std::string &c)
    {
        std::vector<std::string> result;
        std::string::size_type pos1, pos2;
        pos2 = s.find(c);
        pos1 = 0;
        while (std::string::npos != pos2)
        {
            result.emplace_back(s.substr(pos1, pos2 - pos1));

            pos1 = pos2 + c.size();
            pos2 = s.find(c, pos1);
        }
        if (pos1 != s.length())
            result.emplace_back(s.substr(pos1));
        return result;
    }

    std::vector<std::string> SplitMultiDelim(const std::string &s, const std::string &seperator)
    {
        std::vector<std::string> result;
        typedef std::string::size_type string_size;
        string_size i = 0;

        while (i != s.size())
        {
            //找到字符串中首个不等于分隔符的字母；
            int flag = 0;
            while (i != s.size() && flag == 0)
            {
                flag = 1;
                for (string_size x = 0; x < seperator.size(); ++x)
                    if (s[i] == seperator[x])
                    {
                        ++i;
                        flag = 0;
                        break;
                    }
            }

            //找到又一个分隔符，将两个分隔符之间的字符串取出；
            flag = 0;
            string_size j = i;
            while (j != s.size() && flag == 0)
            {
                for (string_size x = 0; x < seperator.size(); ++x)
                    if (s[j] == seperator[x])
                    {
                        flag = 1;
                        break;
                    }
                if (flag == 0)
                    ++j;
            }
            if (i != j)
            {
                result.emplace_back(s.substr(i, j - i));
                i = j;
            }
        }
        return result;
    }

    void TrimHeadTail(std::string &srcBuf, const char ch)
    {
        int idx = srcBuf.find_first_not_of(ch);
        if (-1 != idx)
            srcBuf = srcBuf.substr(idx, srcBuf.size() - idx);
        idx = srcBuf.find_last_not_of(ch);
        if (-1 != idx)
            srcBuf = srcBuf.substr(0, idx + 1);
    }

    uint8_t charToU8(const char *str)
    {
        uint8_t out = 0;
        sscanf(str, "%" SCNu8, &out);
        return out;
    }

    uint16_t charToU16(const char *str)
    {
        uint16_t out = 0;
        sscanf(str, "%" SCNu16, &out);
        return out;
    }

    uint32_t charToU32(const char *str)
    {
        uint32_t out = 0;
        sscanf(str, "%" SCNu32, &out);
        return out;
    }

    uint64_t charToU64(const char *str)
    {
        uint64_t out = 0;
        sscanf(str, "%" SCNu64, &out);
        return out;
    }

    int8_t charToI8(const char *str)
    {
        int8_t out = 0;
        sscanf(str, "%" SCNi8, &out);
        return out;
    }

    int16_t charToI16(const char *str)
    {
        int16_t out = 0;
        sscanf(str, "%" SCNi16, &out);
        return out;
    }

    int32_t charToI32(const char *str)
    {
        int32_t out = 0;
        sscanf(str, "%" SCNi32, &out);
        return out;
    }
    
    int64_t charToI64(const char *str)
    {
        int64_t out = 0;
        sscanf(str, "%" SCNi64, &out);
        return out;
    }
}