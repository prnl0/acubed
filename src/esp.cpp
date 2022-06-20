#include "esp.h"

#include <memory>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <tuple>

#include "rectangle.h"
#include "player.h"
#include "config.h"
#include "aimbot.h"
#include "target.h"
#include "angle.h"
#include "vector.h"
#include "game.h"
#include "entity.h"

#include "imgui/imgui.h"
#include "imgui/imgui_extensions.h"

bool ESP::player_esp_enabled = false;
bool ESP::entity_esp_enabled = false;

Shapes::Rectangle ESP::esp_box;

void ESP::draw_player_esp()
{
    const auto settings = ESPConfig::player_esp_settings;
    const bool saia = IS_SET(settings, ESPConfig::show_add_info_all);
    const bool saits = IS_SET(settings, ESPConfig::show_add_info_targets);
    const bool sait = IS_SET(settings, ESPConfig::show_add_info_target);

    const auto &tlist = TargetManager::get_all();
    const auto target = Aimbot::get_target();

    if (IS_SET(settings, ESPConfig::show_info_target) && target && is_player_valid(target)) {
        /* Always show additional information for a target as they are both in
         * the player list, and the target list. */
        draw_player(target, true);
    } else if (IS_SET(settings, ESPConfig::show_info_targets)) {
        for (const auto &t : tlist) {
            if (!is_player_valid(t.data)) {
                continue;
            }

            /* Show additional information if:
             *     - we want to show it for everyone;
             *     - we want to show it for targets only;
             *     - the player is the current target and we want to show it
             *       for the current target only. */
            draw_player(t.data, saia || saits || (t.data == target && sait));
        }
    } else if (IS_SET(settings, ESPConfig::show_info_all)) {
        for (const auto &p : PlayerList::get_list()) {
            if (!is_player_valid(p)) {
                continue;
            }

            /* Show additional information if:
             *     - we want to show it for everyone;
             *     - the player is in the target list and we want to show it
             *       for targets only;
             *     - the player is the current target and we want to show it
             *       for the current target only. */
            draw_player(p, saia || (tlist.cend() != TargetManager::find(p->get_name()) && saits) ||
                        (p == target && sait));
        }
    }
}

bool ESP::is_player_valid(const Player *p)
{
    const auto &lp = *LocalPlayerConfig::ptr;
    const auto target_angle = Angle::compute_angle(lp.get_position(), p->get_position());

    /* A player is valid if:
     *     - they are alive;
     *     - they are in the local players' FOV;
     *     - we want to show teammates or they are not a teammate. */
    return !p->is_dead() &&
        Angle::is_in_range(lp.get_viewangles()[0], target_angle[0], lp.get_fov()) &&
        (IS_SET(ESPConfig::player_esp_settings, ESPConfig::show_teammates) ||
         lp.get_team() != p->get_team());
}

void ESP::draw_player(const Player *player,
                      bool show_additional_info)
{
    const auto &lp = *LocalPlayerConfig::ptr;

    const auto &lp_pos = lp.get_position();
    const auto &p_pos = player->get_position();
    const float dist = lp_pos.distance(p_pos);

    /* Computes the tangent of the angle between the screen and the center
     * point of the target. */
    const float scale = (ImGui::GetIO().DisplaySize.y / 2) / dist;

    const auto scr_coords = Game::world_to_screen(lp.get_pvm_matrix(),
                                                  {p_pos[0], p_pos[1], p_pos[2] - 4.5f / 2, 1.0f});

    const auto color = player == Aimbot::get_target() ?
                           ESPConfig::box_target_col :
                           player->get_team() == PlayerConstants::Team::blue ?
                               ESPConfig::box_rvsf_col :
                               ESPConfig::box_cla_col;

    draw_esp_box({scr_coords[0], scr_coords[1]}, scale, color, true);

    float y_pos = 0.0f;
    if (IS_SET(ESPConfig::player_esp_settings, ESPConfig::show_hp_bar) ||
        IS_SET(ESPConfig::player_esp_settings, ESPConfig::show_ap_bar)) {
        y_pos = draw_hp_ap_bars(player).bottom_left[1];
    }

    if (IS_SET(ESPConfig::player_esp_settings, ESPConfig::show_reload_bar) &&
        player->is_reloading()) {
        draw_reload_bar(player);
    }

    if (IS_SET(ESPConfig::player_esp_settings, ESPConfig::show_distance)) {
        std::stringstream stream;
        stream << std::fixed << std::setprecision(1) << '(' << dist << ')';
        draw_entity_label({-1.0f, y_pos + 3.0f}, stream.str(), color);
    }

    if (IS_SET(ESPConfig::player_esp_settings, ESPConfig::show_player_name)) {
        if (IS_SET(ESPConfig::player_esp_settings, ESPConfig::show_client_number)) {
            draw_entity_label({-1.0f, -1.0f},
                             (player->get_name() +
                                 '(' + std::to_string(player->get_cn()) + ')'),
                             color);
        } else {
            draw_entity_label({-1.0f, -1.0f}, player->get_name(), color);
        }
    }

    if (show_additional_info && ESPConfig::max_distance >= dist) {
        draw_player_add_info(player, color);
    }
}

Shapes::Rectangle ESP::draw_hp_ap_bars(const Player *player)
{
    const auto draw_list = ImGui::GetCurrentWindow()->DrawList;
    float width = 0.0f;
    Shapes::Rectangle bar;

    /* Draw health bar. */
    if (IS_SET(ESPConfig::player_esp_settings, ESPConfig::show_hp_bar)) {
        const float scale = player->get_health() / 100.0f;
        width = (esp_box.top_right[0] - esp_box.top_left[0]) * scale;

        bar = {
            {esp_box.top_left[0]        , esp_box.bottom_left[1] + 3.0f       },
            {esp_box.top_left[0] + width, esp_box.bottom_left[1] + 3.0f       },
            {esp_box.top_left[0] + width, esp_box.bottom_left[1] + 3.0f + 1.0f},
            {esp_box.top_left[0]        , esp_box.bottom_left[1] + 3.0f + 1.0f}
        };

        ImGui::DrawOutlinedRectangleFilled({bar.top_left[0], bar.top_left[1]},
                                           {bar.bottom_right[0], bar.bottom_right[1]},
                                           IM_COL32(255 * (1 - scale), 255 * scale, 0, 255));
    }

    /* Draw armour bar. */
    if (0 != player->get_armour() &&
        IS_SET(ESPConfig::player_esp_settings, ESPConfig::show_ap_bar)) {
        width = (esp_box.top_right[0] - esp_box.top_left[0]) * (player->get_armour() / 100.0f);
        const float y = (IS_SET(ESPConfig::player_esp_settings,
                                ESPConfig::show_hp_bar) + 1) * 3.0f + 1.0f;

        bar = {
            {esp_box.top_left[0]        , esp_box.bottom_left[1] + y       },
            {esp_box.top_left[0] + width, esp_box.bottom_left[1] + y       },
            {esp_box.top_left[0] + width, esp_box.bottom_left[1] + y + 1.0f},
            {esp_box.top_left[0]        , esp_box.bottom_left[1] + y + 1.0f}
        };

        ImGui::DrawOutlinedRectangleFilled({bar.top_left[0], bar.top_left[1]},
                                           {bar.bottom_right[0], bar.bottom_right[1]},
                                           ESPConfig::ap_bar_col);
    }

    return bar;
}

void ESP::draw_reload_bar(const Player *player)
{
    const float scale = player->reload_time_left() * 1.0f /
                        player->get_current_weapon()->get_attack_delay();
    const float height = (esp_box.top_left[1] - esp_box.bottom_left[1]) * scale;

    Shapes::Rectangle bar{
        {esp_box.top_left[0] - 3.0f - 1.0f, esp_box.bottom_left[1] + height},
        {esp_box.top_left[0] - 3.0f       , esp_box.bottom_left[1] + height},
        {esp_box.top_left[0] - 3.0f       , esp_box.bottom_left[1]         },
        {esp_box.top_left[0] - 3.0f - 1.0f, esp_box.bottom_left[1]         }
    };

    ImGui::DrawOutlinedRectangleFilled({bar.top_left[0], bar.top_left[1]},
                                       {bar.bottom_right[0], bar.bottom_right[1]},
                                       ESPConfig::reload_bar_col);
}

void ESP::draw_player_add_info(const Player *player,
                               ImU32 border_color)
{
    const ImVec2 window_pos{esp_box.top_right[0] + 5.0f, esp_box.top_right[1] + 1.0f};
    ImGui::SetNextWindowPos(window_pos);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, border_color);
    ImGui::Begin(("##" + std::to_string(player->get_cn())).c_str(), nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse |
                 ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar |
                 ImGuiWindowFlags_NoInputs);
        ImGui::BeginGroup();
            const auto settings = ESPConfig::player_esp_settings;
            if (IS_SET(settings, ESPConfig::show_add_health)) {
                ImGui::Text("Health: %d", player->get_health());
            }

            if (IS_SET(settings, ESPConfig::show_add_armour)) {
                ImGui::Text("Armour: %d", player->get_armour());
            }

            if (IS_SET(settings, ESPConfig::show_add_kills)) {
                ImGui::Text("Kills: %d", player->get_kills());
            }

            if (IS_SET(settings, ESPConfig::show_add_deaths)) {
                ImGui::Text("Deaths: %d", player->get_deaths());
            }

            const auto curr_wpn = player->get_current_weapon();
            if (IS_SET(settings, ESPConfig::show_add_weapon)) {
                ImGui::Text("Weapon: %s", curr_wpn->get_name().c_str());
            }

            if (IS_SET(settings, ESPConfig::show_add_ammo)) {
                curr_wpn->get_id() == WeaponConstants::ID::grenade ?
                    ImGui::Text("Grenades: %d", curr_wpn->get_clip()) :
                    ImGui::Text("Ammo: %d/%d", curr_wpn->get_clip(), curr_wpn->get_mag());
            }
        ImGui::EndGroup();
        const auto size = ImGui::GetWindowSize();
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    /* Draw an additional black border around window. */
    const auto draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRect({window_pos.x - 2.0f, window_pos.y - 2.0f},
                       {window_pos.x + size.x + 1.0f, window_pos.y + size.y + 1.0f},
                       IM_COL32(0, 0, 0, 255));
}

void ESP::draw_entity_esp()
{
    const auto lp = *LocalPlayerConfig::ptr;
    const auto &pvm_matrix = lp.get_pvm_matrix();

    const auto settings = ESPConfig::entity_esp_settings;
    const auto &ent_list = EntityList::get_list();

    static const auto draw_entity =
        [&pvm_matrix, &settings, &lp](const Vector<float, 3> &pos,
                                      const ESPConfig::entity_data &data,
                                      const int id = -1) {
        const auto screen_pos = Game::world_to_screen(pvm_matrix, {pos[0], pos[1], pos[2], 1.0f});
        const float dist = lp.get_position().distance(pos);
        const float scale = (ImGui::GetIO().DisplaySize.y / 2) / dist;

        draw_esp_box(screen_pos, scale, std::get<1>(data), false);

        if (IS_SET(settings, ESPConfig::show_ent_name)) {
            if (IS_SET(settings, ESPConfig::show_ent_id) && -1 != id) {
                draw_entity_label({screen_pos[0], -1.0f},
                                  std::get<0>(data) + '(' + std::to_string(id) + ')',
                                  std::get<1>(data));
            } else {
                draw_entity_label({screen_pos[0], -1.0f}, std::get<0>(data), std::get<1>(data));
            }
        }

        if (IS_SET(settings, ESPConfig::show_ent_distance)) {
            std::stringstream stream;
            stream << std::fixed << std::setprecision(1) << '(' << dist << ')';
            draw_entity_label({-1.0f, esp_box.bottom_right[1] + 1.0f}, stream.str(),
                              std::get<1>(data));
        }
    };

    for (decltype(ent_list.size()) i = 0; ent_list.size() != i; ++i) {
        const auto ent = ent_list[i];
        const auto type = ent->get_type();

        /* Don't draw an entity if:
         *     - it isn't shown;
         *     - it is hidden and we don't want to draw hidden entities,
         * or ... */
        if (!IS_SET(settings, 1U << (type - 1)) ||
            (!ent->is_visible() && !IS_SET(settings, ESPConfig::show_hidden))) {
            continue;
        }

        auto pos = ent->get_pos();
        const auto target_angle = Angle::compute_angle(lp.get_position(), pos);

        /* ... it is out of view. */
        if (!Angle::is_in_range(lp.get_viewangles()[0], target_angle[0], lp.get_fov())) {
            continue;
        }

        if (IS_SET(settings, ESPConfig::align_box) && (2 < type && 10 > type)) {
            /* Get ground level below entity. */
            pos = Game::trace(pos, {pos[0], pos[1], pos[2] - 500.0f}, ent).end;
            
            /* Position the ESP box 1.8 units above the ground. */
            pos[2] += 1.8f + ent->get_values()[0];
        }

        draw_entity(pos, ESPConfig::ent_data[type - 1], i);
    }

    for (const auto ent : EntityList::get_dyn_list()) {
        /* Don't draw an entity if it isn't shown, or ... */
        if (!IS_SET(settings, 1U << (15 + ent->get_type()))) {
            continue;
        }

        const auto pos = ent->get_pos();
        const auto target_angle = Angle::compute_angle(lp.get_position(), pos);

        /* ... it is out of view. */
        if (!Angle::is_in_range(lp.get_viewangles()[0], target_angle[0], lp.get_fov())) {
            continue;
        }

        const auto &data = ESPConfig::dyn_ent_data[ent->get_type() - 1];
        draw_entity(pos, data);

        if (IS_SET(settings, ESPConfig::show_despawn_time)) {
            const float y_pos = esp_box.bottom_right[1] + 1.0f + (ImGui::GetFontSize() + 1.0f) *
                                IS_SET(settings, ESPConfig::show_ent_distance);
            draw_entity_label({-1.0f, y_pos},
                              std::to_string(ent->get_time_left()).c_str(), std::get<1>(data));
        }
    }
}

void ESP::draw_entity_label(const Vector<float, 2> &center,
                            const std::string &label,
                            ImU32 color)
{
    const auto text_size = ImGui::CalcTextSize(label.c_str());
    ImVec2 cursor_pos{center[0], center[1]};

    /* If provided with non-positive coordinates, deduce them ourselves. */
    if (0.0f >= center[0]) {
        cursor_pos.x = esp_box.top_left[0] + (esp_box.top_right[0] - esp_box.top_left[0]) / 2;
    }
    cursor_pos.x -= text_size.x / 2 - 1.0f;

    if (0.0f >= center[1]) {
        cursor_pos.y = esp_box.top_left[1] - text_size.y + 1.0f;
    }

    ImGui::GetCurrentWindow()->DC.CursorPos = cursor_pos;

    /* Draw minimal (1px) shadow at the bottom-right corner of the name. */
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
    ImGui::Text(label.c_str());

    /* Reposition cursor and draw the actual name. */
    ImGui::GetCurrentWindow()->DC.CursorPos = {cursor_pos.x - 1.0f, cursor_pos.y - 1.0f};
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::Text(label.c_str());

    ImGui::PopStyleColor(2);
}

void ESP::draw_esp_box(const Vector<float, 2> center,
                       float scale,
                       ImU32 color,
                       bool player_box)
{
    auto &box = esp_box;
    if (player_box) {
        box.top_left        = {center[0] - ESPConfig::player_box_width / 2, center[1] - 2.0f   };
        box.top_right       = {center[0] + ESPConfig::player_box_width / 2, center[1] - 2.0f   };
        box.bottom_right    = {box.top_right[0]                           , center[1] + 8.0f   };
        box.bottom_left     = {box.top_left[0]                            , box.bottom_right[1]};

        box.scale(scale);

        /* Adjust bottom Y position to remove the residual spacing between the
         * bounding box and the players' feet. */
        box.bottom_left[1] = box.bottom_right[1] -= 10.0f;
    } else {
        box.top_left        = {center[0] - ESPConfig::entity_box_width / 2, center[1]          };
        box.top_right       = {center[0] + ESPConfig::entity_box_width / 2, center[1]          };
        box.bottom_right    = {box.top_right[0]                           , center[1] + 1.5f   };
        box.bottom_left     = {box.top_left[0]                            , box.bottom_right[1]};

        box.scale(scale);
    }

    ImGui::DrawOutlinedRectangle({box.top_left[0], box.top_left[1]},
                                 {box.bottom_right[0], box.bottom_right[1]},
                                 color);
}