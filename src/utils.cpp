#include "utils.h"

#include <string>
#include <cctype>

#include "constants.h"

std::string Utils::String::to_upper(const std::string &str)
{
    std::string out;
    for (auto c : str) { out += std::toupper(c); }
    return out;
}

const char *Utils::String::find_last_of(const char *beg,
                                        const char *end,
                                        char c)
{
    const int op = end > beg ? +1 : -1;
    for (auto it = beg; end != it; it += op) {
        if (c == *it) {
            return it;
        }
    }

    return end - 1;
}