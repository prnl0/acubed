#include "player.h"

#include <cstdint>
#include <cstring>

#include "memory.h"
#include "weapon.h"
#include "config.h"
#include "vector.h"
#include "matrix.h"
#include "game.h"
#include "target.h"

bool LocalPlayer::can_see(const Player &p) const
{
    return !Game::trace(position, p.get_position(), this).has_collided;
}

std::vector<Player *> PlayerList::list;

void PlayerList::repopulate()
{
    list.clear();
    TargetManager::invalidate_all();

    const std::int32_t player_count = *reinterpret_cast<std::int32_t *>(
        Config::mod_base_address + Offsets::Module::player_count);
    const std::uintptr_t player_list = *reinterpret_cast<std::uintptr_t *>(
        Config::mod_base_address + Offsets::Module::player_list_pointer);

    for (std::int32_t i = 0; player_count != i; ++i) {
        /* TODO: provide an #ifdef to handle 64-bit pointers. */
        Player **ptr_to_player_addr = reinterpret_cast<Player **>(player_list + 0x4 * i);
        Player *address = nullptr;

        if (ptr_to_player_addr && (address = *ptr_to_player_addr) &&
            LocalPlayerConfig::ptr != address) {
            list.push_back(address);

            /* If a target with this name exists, update their data. */
            TargetManager::set(address->get_name(), address);
        }
    }
}