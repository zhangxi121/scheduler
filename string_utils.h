#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <inttypes.h>
#include <iostream>
#include <string>
#include <vector>

namespace nmsp
{
    std::string &replace_all(std::string &str, const std::string &old_value, const std::string &new_value);
    std::string &replace_all_distinct(std::string &str, const std::string &old_value, const std::string &new_value);
    std::vector<std::string> SplitString(const std::string &s, const std::string &c);
    std::vector<std::string> SplitMultiDelim(const std::string &s, const std::string &seperator);
    void TrimHeadTail(std::string &srcBuf, const char ch = ' ');

    uint8_t charToU8(const char *str);
    uint16_t charToU16(const char *str);
    uint32_t charToU32(const char *str);
    uint64_t charToU64(const char *str);

    int8_t charToI8(const char *str);
    int16_t charToI16(const char *str);
    int32_t charToI32(const char *str);
    int64_t charToI64(const char *str);
}

#endif
