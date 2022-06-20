#pragma once

#ifndef AIMBOT_GUARD
#define AIMBOT_GUARD

#include "player.h"
#include "config.h"

class Aimbot
{
    friend class Menu;

    public:
        Aimbot() = delete;

        static void aim();

        static bool     is_enabled() { return enabled;  }
        static Player  *get_target() { return target;   }

    private:
        static Player *find_target();
        static bool is_player_valid(const Player *p,
                                    float *distance_to_target = nullptr);
        
        static bool enabled;
        static bool prev_attacking_state;

        static Player *target;
};

#endif // ifndef AIMBOT_GUARD