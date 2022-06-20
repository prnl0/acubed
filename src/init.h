#pragma once

#ifndef INIT_GUARD
#define INIT_GUARD

#include <Windows.h>

namespace Init
{
    void init(const HWND &hwnd);
    void shutdown();
} // namespace Init

#endif // INIT_GUARD