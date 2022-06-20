#include "weapon.h"

#include "config.h"
#include "memory.h"
#include "constants.h"

#include <cstddef>
#include <tuple>

void Weapon::toggle_auto_attack_all(bool state)
{
    static const auto weapon_list = reinterpret_cast<Weapon **>(
                                    LocalPlayerConfig::weapon_list_address);
    constexpr auto &auto_attack_defaults = WeaponConstants::auto_attack_defaults;

    if (state) {
        for (std::size_t i = 0; WeaponConstants::weapon_count != i; ++i) {
            weapon_list[i]->set_auto_attack(true);
        }
    } else {
        /* Reset to initial values. */
        for (std::size_t i = 0; WeaponConstants::weapon_count != i; ++i) {
            weapon_list[i]->set_auto_attack(WeaponConstants::auto_attack_defaults[i]);
        }
    }
}

void Weapon::toggle_consec_shots(bool state)
{
    Memory::patch_bytes(reinterpret_cast<void *>(Config::mod_base_address +
                                                 Offsets::PatchRegions::inc_consec_shots),
                        state ? "\xFF\x46\x1C" : "\x90\x90\x90", 3);
}

void Weapon::reset_rsk()
{
    static const auto weapon_list = reinterpret_cast<Weapon **>(LocalPlayerConfig::weapon_list_address);
    constexpr auto &rsk_defaults = WeaponConstants::rsk_defaults;

    for (std::size_t i = 0; WeaponConstants::weapon_count != i; ++i) {
        weapon_list[i]->set_recoil(std::get<0>(rsk_defaults[i]));
        weapon_list[i]->set_spread(std::get<1>(rsk_defaults[i]));
        weapon_list[i]->set_knockback(std::get<2>(rsk_defaults[i]));
    }
}