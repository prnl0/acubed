#pragma once

#ifndef CONFIG_GUARD
#define CONFIG_GUARD

#include <cstdint>  // uintptr_t; uint32_t; uint8_t; int16_t
#include <array>    // array
#include <string>   // string
#include <tuple>    // tuple

#include <Windows.h>

#include "imgui/imgui.h"

#include "vector.h"

/* Macros for bitflag manipulation. */
#define IS_SET(flags, flag) !!(flags & flag)
#define SET(flags, flag) (flags |= flag)
#define UNSET(flags, flag) (flags &= ~flag)

class LocalPlayer;

struct Config
{
    using addr = std::uintptr_t;

    Config() = delete;

    static addr mod_base_address;
    static HWND hwnd;
    static std::uint32_t *ticks;
};

struct MenuConfig : Config
{
    enum Position {
        bottom  = 1UL << 0,
        right   = 1UL << 1
    };

    enum Category {
        aimbot,
        esp,
        triggerbot,
        miscellaneous,
        styling,
        load_save
    };

    enum Flags {
        remember_cursor_pos = 1UL << 0,     // [DEFAULT] Prevents abnormal crosshair repositioning after leaving the menu.

        show_info           = 1UL << 1,     // [DEFAULT] Shows the information window.
        show_player_info    = 1UL << 2,     // [DEFAULT] Shows player-related information.
        show_name           = 1UL << 3,     // [DEFAULT] Shows the players' name.
        show_client_number  = 1UL << 4,     // [DEFAULT] Shows the players' client number.
        show_health         = 1UL << 5,     // [DEFAULT] Shows the players' current health amount.
        show_armour         = 1UL << 6,     // [DEFAULT] Shows the players' current armour amount.
        show_kills          = 1UL << 7,     // [DEFAULT] Shows the amount of kills the player has.
        show_deaths         = 1UL << 8,     // [DEFAULT] Shows the amount of times the player died.
        show_kd_ratio       = 1UL << 9,     // [DEFAULT] Shows the kill-to-death ratio.
        show_weapon_name    = 1UL << 10,    // [DEFAULT] Shows the currently equipped weapons' name.
        show_ammo           = 1UL << 11,    // [DEFAULT] Shows the ammo (clip/magazine) of the current weapon
        show_position       = 1UL << 12,    // [DEFAULT] Shows the players' world position (X; Y; Z).
        show_viewangles     = 1UL << 13,    // [DEFAULT] Shows the players' viewangles (pitch; yaw).
        show_fps            = 1UL << 14     // [DEFAULT] Shows the current FPS.
    };

    static unsigned settings;

    /* Main window */
    static const std::array<std::string, 6> categories;                                 // Main categories displayed on the left side of the menu.
    static constexpr std::array<decltype(categories)::size_type, 1> fillers{3};         // Indexes of fillers (empty space) in-between categories.
    static constexpr ImVec2 category_base_size          = {150.0f, 50.0f};              // Base size of a category selection button.
    static constexpr ImVec2 base_size                   = {720.0f, 450.0f};             // Base size of the main menu window.
    static constexpr float rounding                     = 0.0f;                         // Rounding of the main menu window.
    static constexpr float border_size                  = 1.0f;                         // Border size of the main menu window.
    static constexpr float scrollbar_rounding           = 0.0f;                         // Global scrollbar rounding.
    static constexpr float background_alpha             = 0.8f;                         // Background opacity of the main menu window.
    static constexpr ImVec2 window_padding              = {5.0f, 5.0f};                 // Global padding within windows.
    static constexpr ImVec2 tooltip_padding             = {5.0f, 5.0f};                 // Global padding within tooltips.
    static constexpr ImVec2 panel_padding               = {10.0f, 8.0f};                // Global padding within panels.
    static constexpr auto scrollbar_color               = IM_COL32(41,  74, 122, 255);
    static constexpr auto scrollbar_hover_color         = IM_COL32(32,  57,  96, 255);
    static constexpr auto scrollbar_active_color        = IM_COL32(56, 101, 167, 255);
    static constexpr auto border_color                  = IM_COL32(66, 150, 250, 255);
    static constexpr auto separator_color               = IM_COL32(66, 150, 250, 255);
    static constexpr auto background_color              = IM_COL32( 0,   0,   0, 255);
    static constexpr auto category_color                = IM_COL32(66, 150, 250, 255);
    static constexpr auto category_hovered_color        = IM_COL32(66, 150, 250, 255);
    static constexpr auto category_active_color         = IM_COL32( 5, 116, 250, 255);
    static constexpr auto table_border_color            = IM_COL32(41,  74, 122, 255);
    static constexpr auto table_header_bg_color         = IM_COL32(41,  74, 122, 255);
    static constexpr auto table_header_chkmrk_color     = IM_COL32( 0,  23,  56, 134);
    static constexpr auto table_row_color               = IM_COL32(11,  73, 145,  79);
    static constexpr auto table_row_hover_color         = IM_COL32(18,  77, 147, 137);
    static constexpr auto table_row_active_color        = IM_COL32(20,  67, 123, 255);
    static constexpr auto table_row_twin_color          = IM_COL32( 0,  28,  60, 128);
    static constexpr auto table_row_max_targets_color   = IM_COL32( 0,   0,   0, 179);

    /* Information window */
    static constexpr float info_border_size         = 1.0f;                         // Border size of the information window.
    static constexpr float info_rounding            = 0.0f;                         // Rounding of the information window.
    static constexpr float info_background_alpha    = 0.8f;                         // Background opacity of the information window.
    static constexpr ImVec2 info_padding            = {10.0f, 10.0f};               // Padding within the information window.
    static constexpr auto info_border_color         = IM_COL32(66, 150, 250, 255);  // Border color of the information window.
    static std::uint8_t info_pos;                                                   // [DF:bottom|right] Sets the position of the information window.

    /* Player information window */
    static constexpr ImVec2 player_info_base_size       = {800.0f, 500.0f}; // Base size of the player information window.
    static constexpr ImVec2 player_info_padding         = {35.0f, 20.0f};   // Padding within the player information window.
    static constexpr unsigned player_info_rows          = 14;               // The minimum amount of rows to be displayed before a scrollbar appears.
    static constexpr float player_info_background_alpha = 0.8f;             // Background opacity of the player information window.
};

struct LocalPlayerConfig : Config
{
    /* Defined here because it is used fairly often in various places. */
    static LocalPlayer *ptr;

    static addr viewmatrix_address;
    static addr fov_address;
    static addr show_blocked_icon_address;
    static addr call_viewangle_mod_address;
    static addr weapon_list_address;
};

struct AimbotConfig : Config
{
    enum Flags {
        lock_on_target          = 1UL << 0, // [DEFAULT] Maintain target until they are dead or out of the FOE.
        auto_aim                = 1UL << 1, // [DEFAULT] Do not wait for user input to aim - always scan for new targets in the FOE.
        auto_attack             = 1UL << 2, // [DEFAULT] Automatically attack the target.
        aim_through_walls       = 1UL << 3,
        aim_at_teammates        = 1UL << 4,
        hide_blocked            = 1UL << 5, // [DEFAULT] Hide the crossed circle ("Blocked" icon) when aiming at a teammate.
        disable_mouse           = 1UL << 6, // [DEFAULT] Disable mouse when the aimbot is active (a target is in the FOE).
        disable_on_reload       = 1UL << 7, // [DEFAULT] Disable aimbot on weapon reload.
        smoothing_enabled       = 1UL << 8  // Prevent immediate snapping to target by instead moving the players' viewangles in steps, whose size
                                            // is determined by the "Smoothing" variable.
    };

    /* Only one may be on at any single point in time. */
    enum TargetFlags {
        prioritize_targets  = 1UL << 0, // [DEFAULT] Prioritize targets over regular players when selecting the main target.
        aim_at_targets_only = 1UL << 1  // Isolate targets - do not aim at regular players.
    };

    static unsigned settings;
    static unsigned target_settings;

    static float foe;       // [DEFAULT: 90.0f] Field of Effect - the amount of degrees to scan around the local player for targets.
    static float smoothing; // [DEFAULT: 1.0f] The amount of steps needed to finish aiming at a target.

    static bool can_aim; // Controls when to aim at a player when ``auto_aim`` is not enabled.
};

struct ESPConfig : Config
{
    enum PlayerFlags {
        show_enemies            = 1U << 0,  // [DEFAULT]
        show_teammates          = 1U << 1,
        show_player_name        = 1U << 2,  // [DEFAULT]
        show_client_number      = 1U << 3,  // [DEFAULT]
        show_distance           = 1U << 4,
        show_hp_bar             = 1U << 5,  // [DEFAULT]
        show_ap_bar             = 1U << 6,  // [DEFAULT]
        show_reload_bar         = 1U << 7,  // [DEFAULT]
        show_info_all           = 1U << 8,  // [DEFAULT] Show basic information (listed above) for everyone.
        show_info_targets       = 1U << 9,  // Show basic information for targets only.
        show_info_target        = 1U << 10, // Show basic information for the current target only.
        show_add_info_all       = 1U << 11, // [DEFAULT] Show additional information (listed below) for everyone.
        show_add_info_targets   = 1U << 12, // Show additional information for targets only.
        show_add_info_target    = 1U << 13, // Show additional information for the current target only.
        show_add_health         = 1U << 14, // [DEFAULT]
        show_add_armour         = 1U << 15, // [DEFAULT]
        show_add_kills          = 1U << 16, // [DEFAULT]
        show_add_deaths         = 1U << 17, // [DEFAULT]
        show_add_weapon         = 1U << 18, // [DEFAULT]
        show_add_ammo           = 1U << 19  // [DEFAULT]
    };

    enum EntityFlags {
        /* Static entities */
        show_light          = 1U << 0,
        show_playerstart    = 1U << 1,
        show_pistol         = 1U << 2,
        show_ammobox        = 1U << 3,
        show_grenade        = 1U << 4, // [DEFAULT]
        show_health         = 1U << 5, // [DEFAULT]
        show_helmet         = 1U << 6,
        show_kevlar         = 1U << 7,
        show_akimbo         = 1U << 8,
        show_mapmodel       = 1U << 9,
        show_trigger        = 1U << 10,
        show_ladder         = 1U << 11,
        show_ctf_flag       = 1U << 12,
        show_sound          = 1U << 13,
        show_clip           = 1U << 14,
        show_pclip          = 1U << 15,
        
        /* Dynamic entities */
        show_dyn_grenade    = 1U << 16, // [DEFAULT] Show dynamic (thrown) grenades.
        show_gibs           = 1U << 17, // Show gibs ("meat" of a player when knifed/headshot).
        
        /* General settings */
        show_hidden         = 1U << 18, // Show hidden entities (lights, playerstarts, etc.)
        show_ent_name       = 1U << 19, // [DEFAULT]
        show_ent_id         = 1U << 20,
        show_ent_distance   = 1U << 21,
        align_box           = 1U << 22, // [DEFAULT] Align ESP box with model.
        show_despawn_time   = 1U << 23  // [DEFAULT] Show time left until a dynamic entity despawns.
    };

    using entity_data = std::tuple<const std::string, ImU32>;

    static unsigned player_esp_settings;
    static unsigned entity_esp_settings;

    static float max_distance; // [DEFAULT: 31.0f] The maximum distance the additional information window is shown for.

    static float player_box_width; // [DEFAULT: 5.0f]
    static float entity_box_width; // [DEFAULT: 1.5f]

    static std::array<entity_data, 16> ent_data;
    static std::array<entity_data, 2> dyn_ent_data;

    static ImU32 box_cla_col;       // [DEFAULT: 255, 0, 0, 200]
    static ImU32 box_rvsf_col;      // [DEFAULT: 66, 150, 250, 255]
    static ImU32 box_target_col;    // [DEFAULT: 0, 255, 0, 255]
    static ImU32 ap_bar_col;        // [DEFAULT: 241, 242, 235, 255]
    static ImU32 reload_bar_col;    // [DEFAULT: 255, 255, 255, 255]
};

struct MiscConfig : Config
{
    enum FlyingMode {
        ghost,              // Spectate-like.
        inverse_minecraft,  // Minecraft-like with inversed and obscure controls.
        water,              // Puts the player in make-belief water.
        noclip              // Allows the player to phase through walls.
    };

    static unsigned settings;

    static Vector<float, 3> fullbright_col; // [DEFAULT: 255, 255, 255]

    static std::int16_t recoil;
    static std::int16_t spread;
    static std::int16_t knockback;

    static FlyingMode flying_mode; // [DEFAULT: ghost]
    static const std::array<std::string, 4> fly_mode_names;
};

#endif // ifndef CONFIG_GUARD