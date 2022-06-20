#pragma once

#ifndef PLAYER_GUARD
#define PLAYER_GUARD

#include <cstdint>
#include <string>
#include <vector>

#include "vector.h"
#include "config.h"
#include "constants.h"
#include "weapon.h"
#include "matrix.h"
#include "memory.h"

class Player
{
    public:
        Player() = delete;

        Weapon *get_current_weapon() { return weapon; }
        
        std::string             get_name()              const { return name;            }
        std::int32_t            get_cn()                const { return client_number;   }
        std::int32_t            get_health()            const { return health;          }
        std::int32_t            get_armour()            const { return armour;          }
        std::int32_t            get_kills()             const { return kills;           }
        std::int32_t            get_deaths()            const { return deaths;          }
        const Weapon           *get_current_weapon()    const { return weapon;          }
        const Vector<float, 3> &get_position()          const { return position;        }
        const Vector<float, 3> &get_viewangles()        const { return viewangles;      }
        std::int32_t            get_team()              const { return team;            }
        std::int32_t            is_dead()               const { return 1 == state2;     }

        bool is_reloading() const
        {
            /* Isolate the knife case as we cannot reload a knife. The same goes for a
             * grenade but it doesn't set the delay to ridiculous numbers so there's no
             * need to handle it for now.
             *
             * FIXME/NOTE [1]: this is possibly not the best way to detect reloading
             *                 but it works so it'll stay as is for the time being. */
            return weapon->get_attack_delay() >= weapon->get_reload_delay() &&
                   1.1f > reload_time_left() * 1.0f / weapon->get_attack_delay() &&
                   WeaponConstants::ID::knife != weapon->get_id();
        }

        std::uint32_t reload_time_left() const
        {
            return weapon->get_attack_delay() - (*Config::ticks - weapon->get_reload_start());
        }

    protected:
        std::uint8_t padding_0x0[4];        // 0x0   -> 0x4   (4 bytes)
        Vector<float, 3> position;          // 0x4   -> 0x10  (6 bytes)
        std::uint8_t padding_0x10[48];      // 0x10  -> 0x40  (48 bytes)
        Vector<float, 3> viewangles;        // 0x40  -> 0x4C  (12 bytes)
        std::uint8_t padding_0x4C[16];      // 0x4C  -> 0x5C  (16 bytes)
        float player_height;                // 0x5C  -> 0x60  (4 bytes)
        std::uint8_t padding_0x60[34];      // 0x60  -> 0x82  (34 bytes)
        std::uint8_t state1;                // 0x82  -> 0x83  (1 byte)
        std::uint8_t padding_0x83[117];     // 0x83  -> 0xF8  (117 bytes)
        std::int32_t health;                // 0xF8  -> 0xFC  (4 bytes)
        std::int32_t armour;                // 0xFC  -> 0x100 (4 bytes)
        std::uint8_t padding_0x100[228];    // 0x100 -> 0x1E4 (228 bytes)
        std::int32_t client_number;         // 0x1E4 -> 0x1E8 (4 bytes)
        std::uint8_t padding_0x1E8[8];      // 0x1E8 -> 0x1F0 (8 bytes)
        std::int32_t ping;                  // 0x1F0 -> 0x1F4 (4 bytes)
        std::uint8_t padding_0x1F4[8];      // 0x1F4 -> 0x1FC (8 bytes)
        std::int32_t kills;                 // 0x1FC -> 0x200 (4 bytes)
        std::uint8_t padding_0x200[4];      // 0x200 -> 0x204 (4 bytes)
        std::int32_t deaths;                // 0x204 -> 0x208 (4 bytes)
        std::uint8_t padding_0x208[28];     // 0x208 -> 0x224 (28 bytes)
        std::uint8_t is_attacking;          // 0x224 -> 0x225 (1 byte)
        char name[16];                      // 0x225 -> 0x235 (16 bytes)
        std::uint8_t padding_0x235[247];    // 0x235 -> 0x32C (247 bytes)
        std::int32_t team;                  // 0x32C -> 0x330 (4 bytes)
        std::uint8_t padding_0x330[8];      // 0x330 -> 0x338 (8 bytes)
        std::uint32_t state2;               // 0x338 -> 0x33C (4 bytes)
        std::uint8_t padding_0x33C[56];     // 0x33C -> 0x374 (56 bytes)
        Weapon *weapon;                     // 0x374 -> 0x378 (4 bytes)
};

class LocalPlayer : public Player
{
    public:
        LocalPlayer() = delete;

        static void toggle_blocked_icon_visibility(bool visible)
        {
            *reinterpret_cast<std::uint32_t *>(LocalPlayerConfig::show_blocked_icon_address) =
                visible;
        }

        /* Prevent AssaultCube from altering the pitch and yaw of the local
         * player by patching the viewangle modification function call. */
        static void toggle_viewangle_mod(bool state)
        {
            Memory::patch_bytes(
                reinterpret_cast<void *>(LocalPlayerConfig::call_viewangle_mod_address),
                state ? "\xE8\x1A\x69\xFE\xFF" : "\x90\x90\x90\x90\x90", 5);
        }
        
        void set_viewangles(const Vector<float, 3> &new_angles) { viewangles = new_angles;  }
        void set_is_attacking(bool state)                       { is_attacking = state;     }
        void set_state1(std::uint32_t state)                    { state1 = state;           }
        void set_state2(std::uint32_t state)                    { state2 = state;           }

        const auto &get_pvm_matrix()    const { return *reinterpret_cast<Matrix<float, 4, 4> *>(
                                                       LocalPlayerConfig::viewmatrix_address);    }
        float       get_fov()           const { return *reinterpret_cast<float *>(
                                                       LocalPlayerConfig::fov_address);           }

        bool can_see(const Player &p) const;
};

class PlayerList
{
    public:
        PlayerList() = delete;

        static void repopulate();
        static void clear() { list.clear(); }

        static const std::vector<Player *> &get_list() { return list; }

    private:
        static std::vector<Player *> list;
};

#endif // PLAYER_GUARD