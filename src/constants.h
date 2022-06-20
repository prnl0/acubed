#pragma once

#ifndef CONSTANTS_GUARD
#define CONSTANTS_GUARD

#include <string>   // string::size_type
#include <cstddef>  // size_t
#include <tuple>    // tuple
#include <cstdint>  // int16_t
#include <array>    // array

namespace PlayerConstants
{
    constexpr std::string::size_type max_player_name_length = 17;
    constexpr std::size_t max_targets                       = 100;

    enum Team {
        red         = 0,
        blue        = 1,
        cla         = 2,
        rvsf        = 3,
        spectator   = 4
    };
} // namespace PlayerConstants

namespace WeaponConstants
{
    using rsk_data = std::tuple<std::int16_t, std::int16_t, std::int16_t>;

    constexpr std::string::size_type max_weapon_name_length = 17;
    constexpr unsigned weapon_count                         = 10;

    constexpr std::array<bool, 10> auto_attack_defaults{
        true,   // Knife            (ID: 0)
        false,  // Pistol           (ID: 1)
        false,  // Carbine          (ID: 2)
        false,  // Combat shotgun   (ID: 3)
        true,   // Submachine gun   (ID: 4)
        false,  // Sniper rifle     (ID: 5)
        true,   // Assault rifle    (ID: 6)
        false,  // CPistol          (ID: 7)
        false,  // Grenade          (ID: 8)
        true    // Dual pistol      (ID: 9)
    };

    /* Recoil, spread, knockback */
    constexpr std::array<rsk_data, 10> rsk_defaults{
        rsk_data{  0,  1,  1},  // Knife            (ID: 0)
        rsk_data{ 58, 53, 10},  // Pistol           (ID: 1)
        rsk_data{ 60, 10, 60},  // Carbine          (ID: 2)
        rsk_data{140,  1, 35},  // Combat shotgun   (ID: 3)
        rsk_data{ 50, 45, 15},  // Submachine gun   (ID: 4)
        rsk_data{ 85, 50, 50},  // Sniper rifle     (ID: 5)
        rsk_data{ 50, 18, 30},  // Assault rifle    (ID: 6)
        rsk_data{ 50, 35, 10},  // CPistol          (ID: 7)
        rsk_data{  0,  1,  1},  // Grenade          (ID: 8)
        rsk_data{ 25, 50, 10}   // Dual pistol      (ID: 9)
    };

    enum ID {
        knife,
        pistol,
        carbine,
        combat_shotgun,
        submachine_gun,
        sniper_rifle,
        assault_rifle,
        unnamed,
        grenade,
        dual_pistol
    };
} // namespace WeaponConstants

namespace EntityConstants
{
    enum Type {
        null,
        light,
        playerstart,
        pistol_ammo,
        rifle_ammo_box,
        grenade,
        health,
        helmet,
        kevlar,
        akimbo,
        mapmodel,
        trigger,
        ladder,
        ctf_flag,
        sound,
        clip,
        pclip
    };
} // namespace EntityConstants

namespace Offsets
{
    enum Module {
        clients_rad                 = 0xEE250,
        viewmatrix                  = 0x101AE8,
        draw_stencil_shadows        = 0x109B25,
        fullbright_num              = 0x105BA8, // Inaccurate naming.
        water_level                 = 0x105F4C,
        ticks                       = 0x109EAC,
        fullbright_list             = 0x10A1F8, // Inaccurate naming.
        fov                         = 0x10F1C4,
        show_blocked_icon           = 0x10F218,
        local_player_pointer        = 0x10F4F4,
        player_list_pointer         = 0x10F4F8,
        max_players                 = 0x10F4FC,
        player_count                = 0x10F500,
        entity_list_pointer         = 0x110118,
        entity_count                = 0x110120,
        dynamic_entity_list_pointer = 0x110A28,
        dynamic_entity_count        = 0x110A30,
        fly_speed                   = 0x110148
    };

    enum Player {
        head_position           = 0x4,
        feet_position           = 0x34,
        viewangles              = 0x40,
        health                  = 0xF8,
        armour                  = 0xFC,
        client_number           = 0x1E4,
        kills                   = 0x1FC,
        deaths                  = 0x204,
        is_attacking            = 0x224,
        pname                   = 0x225,
        team                    = 0x32C,
        state                   = 0x338,
        weapon_list             = 0x348,
        current_weapon_pointer  = 0x374
    };

    enum Weapon {
        id                  = 0x4,
        info_pointer        = 0xC,
        mag_pointer         = 0x10,
        clip_pointer        = 0x14,
        shoot_delay_pointer = 0x18
    };

    enum WeaponInfo {
        wname           = 0x0,
        reload_delay    = 0x108,
        knockback       = 0x116,
        can_auto_attack = 0x128
    };

    /* TODO: convert to offsets instead of storing fixed addresses. */
    enum Function {
        render_clients  = 0x4157D0,
        send_msg        = 0x4204B0, // Currently unused.
        on_gamestart    = 0x427800,
        traceline       = 0x48A310,
    };

    enum PatchRegions {
        call_renderclients  = 0x7618,
        gldisable_gl_fog    = 0x76FC,
        disable_flying      = 0x58DCA,
        inc_consec_shots    = 0x6370D,
        call_viewangle_mod  = 0x75D71
    };
} // namespace Offsets

#endif // ifndef CONSTANTS_GUARD