#pragma once

#ifndef TARGET_GUARD
#define TARGET_GUARD

#include <string>
#include <vector>

/* Forward declarations. */
class Player;

struct Target
{
    /*
     * Keep a seperate copy of the name because ``data`` may become invalid
     * while still possibly needing to display the connection state of the
     * target in the menu.
     */
    std::string name;
    Player *data = nullptr;
};

class TargetManager
{
    private:
        using value_type = std::vector<Target>;

    public:
        TargetManager() = delete;
        
        static void add(const Target &t) { targets.push_back(t); }
        static bool set(const std::string &t, Player *data);

        static void remove(const std::string &name);
        static void remove(value_type::const_iterator pos) { targets.erase(pos); }

        static value_type::iterator find(const std::string &name);

        static value_type &get_all() { return targets; }
        static void invalidate_all() { for ( auto &t : targets ) { t.data = nullptr; } }

    private:
        static value_type targets;
};

#endif // TARGET_GUARD