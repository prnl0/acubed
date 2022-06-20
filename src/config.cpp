#include "config.h"

#include <cstdint>  // uint32_t; uint8_t; int16_t
#include <array>    // array
#include <string>   // string

#include <Windows.h>

#include "imgui/imgui.h"

#include "vector.h"

class LocalPlayer;

/* General configuration */
Config::addr    Config::mod_base_address    = 0;
HWND            Config::hwnd                = nullptr;
std::uint32_t  *Config::ticks               = nullptr;

LocalPlayer *LocalPlayerConfig::ptr                         = nullptr;
Config::addr LocalPlayerConfig::viewmatrix_address          = 0;
Config::addr LocalPlayerConfig::fov_address                 = 0;
Config::addr LocalPlayerConfig::show_blocked_icon_address   = 0;
Config::addr LocalPlayerConfig::call_viewangle_mod_address  = 0;
Config::addr LocalPlayerConfig::weapon_list_address         = 0;

/* Menu configuration */
const std::array<std::string, 6> MenuConfig::categories = {"aimbot", "esp", "triggerbot", "miscellaneous", "styling", "load/save"};
unsigned MenuConfig::settings                           = remember_cursor_pos | show_info | show_player_info | show_name | show_client_number | show_health | show_armour | show_kills | show_deaths | show_kd_ratio | show_weapon_name | show_ammo | show_position | show_viewangles | show_fps; 
std::uint8_t MenuConfig::info_pos                       = Position::bottom | Position::right;

/* Aimbot configuration */
unsigned AimbotConfig::settings         = lock_on_target | auto_aim | auto_attack | disable_mouse | disable_on_reload | hide_blocked;
unsigned AimbotConfig::target_settings  = prioritize_targets;
float AimbotConfig::foe                 = 90.0f;
float AimbotConfig::smoothing           = 1.0f;
bool AimbotConfig::can_aim              = false;

/* ESP configuration */
unsigned ESPConfig::player_esp_settings = show_enemies | show_player_name | show_client_number | show_hp_bar | show_ap_bar | show_reload_bar | show_info_all | show_add_info_all | show_add_health | show_add_armour | show_add_kills | show_add_deaths | show_add_weapon | show_add_ammo;
unsigned ESPConfig::entity_esp_settings = show_ent_name | align_box | show_despawn_time | show_health | show_grenade | show_dyn_grenade;
float ESPConfig::max_distance           = 31.0f;
float ESPConfig::player_box_width       = 5.0f;
float ESPConfig::entity_box_width       = 1.5f;
std::array<ESPConfig::entity_data, 16> ESPConfig::ent_data = {
    entity_data{"light",        IM_COL32(255, 255, 255, 255)},
    entity_data{"playerstart",  IM_COL32(  0, 255,   0, 255)},
    entity_data{"pistol",       IM_COL32(255,   0,   0, 255)},
    entity_data{"ammobox",      IM_COL32(255,   0,   0, 255)},
    entity_data{"grenade",      IM_COL32(255,   0,   0, 255)},
    entity_data{"health",       IM_COL32(255, 242,   0, 255)},
    entity_data{"helmet",       IM_COL32(255, 242,   0, 255)},
    entity_data{"kevlar",       IM_COL32(255, 242,   0, 255)},
    entity_data{"akimbo",       IM_COL32(255, 242,   0, 255)},
    entity_data{"mapmodel",     IM_COL32(255,   0, 216, 255)},
    entity_data{"trigger",      IM_COL32(  0,   0, 255, 255)},
    entity_data{"ladder",       IM_COL32(126,  26, 204, 255)},
    entity_data{"ctf-flag",     IM_COL32(148,  26,  26, 255)},
    entity_data{"sound",        IM_COL32(255,   0, 216, 255)},
    entity_data{"clip",         IM_COL32(255, 242,   0, 255)},
    entity_data{"pclip",        IM_COL32(255,   0, 216, 255)}
};
std::array<ESPConfig::entity_data, 2> ESPConfig::dyn_ent_data = {
    entity_data{"grenade",  IM_COL32(255, 0, 0, 255)},
    entity_data{"gibs",     IM_COL32(255, 0, 0, 255)}
};
ImU32 ESPConfig::box_cla_col    = IM_COL32(255, 0, 0, 200);
ImU32 ESPConfig::box_rvsf_col   = IM_COL32(66, 150, 250, 255);
ImU32 ESPConfig::box_target_col = IM_COL32(0, 255, 0, 255);
ImU32 ESPConfig::ap_bar_col     = IM_COL32(241, 242, 235, 255);
ImU32 ESPConfig::reload_bar_col = IM_COL32(255, 255, 255, 255);

/* Miscellaneous configuration */
unsigned MiscConfig::settings                               = 0;
Vector<float, 3> MiscConfig::fullbright_col                 = {1.0f, 1.0f, 1.0f};
std::int16_t MiscConfig::recoil                             = 0;
std::int16_t MiscConfig::spread                             = 0;
std::int16_t MiscConfig::knockback                          = 0;
MiscConfig::FlyingMode MiscConfig::flying_mode              = MiscConfig::ghost;
const std::array<std::string, 4> MiscConfig::fly_mode_names = {"ghost", "inverse Minecraft", "water", "noclip"};