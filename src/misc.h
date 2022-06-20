#pragma once

#ifndef MISC_GUARD
#define MISC_GUARD

#include "config.h"

class Misc
{
    friend class Menu;

    public:
        Misc() = delete;
        
        static void toggle_flying(bool state,
                                  MiscConfig::FlyingMode mode);
        static void toggle_wallhack(bool state);

        static bool is_fullbright() { return fullbright;    }
        static bool is_flying()     { return flying;        }

    private:
        static void toggle_flying_internal(bool state,
                                           MiscConfig::FlyingMode mode);

        static bool fullbright;
        static bool rsk;
        static bool flying;
        static bool wallhack;
        static bool chams;
};

#endif // ifndef MISC_GUARD