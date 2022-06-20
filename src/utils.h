#pragma once

#ifndef UTILS_GUARD
#define UTILS_GUARD

#include <string>

#include "vector.h"
#include "player.h"

namespace Utils
{
    namespace String
    {
        std::string to_upper(const std::string &str);

        const char *find_last_of(const char *beg,
                                 const char *end,
                                 char c);
    } // namespace String
} // namespace Utils

#endif // UTILS_GUARD