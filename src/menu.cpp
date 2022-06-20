#include "menu.h"

#include <algorithm>
#include <vector>
#include <utility>
#include <numeric>

#include <Windows.h>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_extensions.h"

#include "constants.h"
#include "config.h"
#include "aimbot.h"
#include "esp.h"
#include "misc.h"
#include "player.h"
#include "target.h"
#include "utils.h"
#include "memory.h"
#include "game.h"

bool Menu::enabled = false;
bool Menu::show = false;

decltype(MenuConfig::categories)::size_type Menu::selected_index = 0;
ImVec2 Menu::scale;
ImVec2 Menu::size;

WNDPROC Menu::old_wndproc = nullptr;

int (*Menu::SDL_ShowCursor)(int) = nullptr;

bool Menu::enable()
{
    if (enabled) {
        return true;
    }

    init_addresses();

    ::SetLastError(0);
    old_wndproc =
        reinterpret_cast<WNDPROC>(::SetWindowLongPtr(Config::hwnd,
                                                     GWL_WNDPROC,
                                                     reinterpret_cast<LONG_PTR>(h_WindowProc)));
    if (!old_wndproc && ::GetLastError()) {
        return false;
    }

    enabled = true;
    return true;
}

bool Menu::disable()
{
    if (!enabled) {
        return true;
    }

    toggle(false);
    SDL_ShowCursor(0); // Remove the residual cursor when resetting the WindowProc function.
    enabled = false;
    return ::SetWindowLongPtr(Config::hwnd, GWL_WNDPROC, reinterpret_cast<LONG_PTR>(old_wndproc));
}

void Menu::generate_drawlist()
{
    if (!enabled) {
        return;
    }

    if (MenuConfig::show_info) {
        generate_info_window();
    }

    if (!show) {
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, MenuConfig::scrollbar_rounding);
    generate_main_window();
    ImGui::PopStyleVar();
}

void Menu::generate_info_window()
{
    const ImGuiIO &io = ImGui::GetIO();
    const ImVec2 window_pos{(MenuConfig::info_pos & MenuConfig::Position::right) ?
                                io.DisplaySize.x - MenuConfig::info_padding[0] :
                                MenuConfig::info_padding.x,
                            (MenuConfig::info_pos & MenuConfig::Position::bottom) ?
                                io.DisplaySize.y - MenuConfig::info_padding[1] :
                                MenuConfig::info_padding.y};
    const ImVec2 pivot{!!(MenuConfig::info_pos & MenuConfig::Position::right) * 1.0f,
                       !!(MenuConfig::info_pos & MenuConfig::Position::bottom) * 1.0f};

    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, pivot);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, MenuConfig::info_border_size);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, MenuConfig::info_rounding);

    ImGui::PushStyleColor(ImGuiCol_Border, MenuConfig::info_border_color);

    ImGui::BeginTransparent("player_info", MenuConfig::info_background_alpha);
        const auto &settings = MenuConfig::settings;
        if (IS_SET(settings, MenuConfig::show_player_info)) {
            const auto lp = LocalPlayerConfig::ptr;

            if (IS_SET(settings, MenuConfig::show_name)) {
                ImGui::Text("Name: %s", lp->get_name().c_str());
            }

            if (IS_SET(settings, MenuConfig::show_client_number)) {
                ImGui::Text("Client number: %d", lp->get_cn());
            }

            if (IS_SET(settings, MenuConfig::show_health)) {
                ImGui::Text("Health: %d", lp->get_health());
            }

            if (IS_SET(settings, MenuConfig::show_armour)) {
                ImGui::Text("Armour: %d", lp->get_armour());
            }

            if (IS_SET(settings, MenuConfig::show_kills)) {
                ImGui::Text("Kills: %d", lp->get_kills());
            }

            if (IS_SET(settings, MenuConfig::show_deaths)) {
                ImGui::Text("Deaths: %d", lp->get_deaths());
            }

            if (IS_SET(settings, MenuConfig::show_kd_ratio)) {
                ImGui::Text("K/D ratio: %.2f",
                            lp->get_deaths() ? 1.0f * lp->get_kills() / lp->get_deaths() : 0.0f);
            }

            const auto curr_wpn = lp->get_current_weapon();
            if (IS_SET(settings, MenuConfig::show_weapon_name)) {
                ImGui::Text("Weapon: %s", curr_wpn->get_name().c_str());
            }

            if (IS_SET(settings, MenuConfig::show_ammo) &&
                WeaponConstants::ID::knife != curr_wpn->get_id()) {
                curr_wpn->get_id() == WeaponConstants::ID::grenade ?
                    ImGui::Text("Grenades: %d", curr_wpn->get_clip()) :
                    ImGui::Text("Ammo: %d/%d", curr_wpn->get_clip(), curr_wpn->get_mag());
            }

            if (IS_SET(settings, MenuConfig::show_position)) {
                const auto pos = lp->get_position();
                ImGui::Text("Position: (%.1f; %.1f; %.1f)", pos[0], pos[1], pos[2]);
            }

            if (IS_SET(settings, MenuConfig::show_viewangles)) {
                const auto viewangles = lp->get_viewangles();
                ImGui::Text("Viewangles: (%.1f; %.1f)", viewangles[0], viewangles[1]);
            }
        }

        if (IS_SET(settings, MenuConfig::show_fps)) {
            if (IS_SET(settings, MenuConfig::show_player_info)) {
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
            }

            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        }
    ImGui::EndTransparent();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

void Menu::generate_main_window()
{
    const auto &io = ImGui::GetIO();

    /* The menu is adapted for 1280x720 resolutions so scale it accordingly. */
    scale = {io.DisplaySize.x / 1280.0f, io.DisplaySize.y / 720.0f};
    size = {scale.x * MenuConfig::base_size.x, scale.y * MenuConfig::base_size.y};
    ImGui::SetNextWindowPos({io.DisplaySize.x / 2, io.DisplaySize.y / 2}, ImGuiCond_Once, {0.5f, 0.5f});
    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowBgAlpha(MenuConfig::background_alpha);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, MenuConfig::rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, MenuConfig::rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, MenuConfig::border_size);

    ImGui::PushStyleColor(ImGuiCol_Border, MenuConfig::border_color);
    ImGui::PushStyleColor(ImGuiCol_Separator, MenuConfig::separator_color);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, MenuConfig::background_color);
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, MenuConfig::scrollbar_color);
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, MenuConfig::scrollbar_hover_color);
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, MenuConfig::scrollbar_active_color);

    ImGui::Begin("main_window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove);
        /* Remove padding within window. */
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.0f, 0.0f});

        /* Using columns here because I had already adapted them before the use
         * of tables and they work decently well. */
        ImGui::BeginColumns(0, 2, ImGuiColumnsFlags_NoResize);
            generate_categories_sub();
        ImGui::PopStyleVar();
        ImGui::NextColumn();
            /* Add artificial padding within the main window in the second
             * column. */
            ImGui::GetCurrentWindow()->DC.CursorPos.y += MenuConfig::window_padding.y * 2.0f;

            /* Reduce the height of the scrollable area to add padding at the
             * bottom. */
            ImGui::BeginChild("##content", {0.0f, -MenuConfig::window_padding.y * 3.0f});
                ImGui::Indent(MenuConfig::window_padding.x);
                switch (selected_index) {
                    case MenuConfig::Category::aimbot:          { generate_aimbot_sub();        break; }
                    case MenuConfig::Category::esp:             { generate_esp_sub();           break; }
                    case MenuConfig::Category::triggerbot:      { generate_triggerbot_sub();    break; }
                    case MenuConfig::Category::miscellaneous:   { generate_misc_sub();          break; }
                    case MenuConfig::Category::styling:         { generate_styling_sub();       break; }
                    case MenuConfig::Category::load_save:       { generate_load_save_sub();     break; }
                    default:                                    {                               break; }
                }
                ImGui::Dummy({0.0f, MenuConfig::window_padding.y * 2.0f});
            ImGui::EndChild();
            ImGui::Dummy({0.0f, MenuConfig::window_padding.y * 2.0f});
        ImGui::EndColumns();
    ImGui::End();

    ImGui::PopStyleColor(6);
    ImGui::PopStyleVar(4);
}

void Menu::generate_player_info_window(bool *in_selection,
                                       std::vector<Target> *targets)
{
    const ImVec2 button_size{60.0f * scale.x, 25.0f * scale.y};

    const ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowPos({io.DisplaySize.x / 2, io.DisplaySize.y / 2}, ImGuiCond_Once, {0.5f, 0.5f});
    ImGui::SetNextWindowSize({scale.x * MenuConfig::player_info_base_size.x,
                             ImGui::GetFrameHeight() * MenuConfig::player_info_rows * 1.25f +
                             button_size.y / 2});
    ImGui::SetNextWindowBgAlpha(MenuConfig::player_info_background_alpha);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, MenuConfig::player_info_padding);

    static std::vector<Player *> selections;
    const auto &plist = PlayerList::get_list();
    const auto &max_targets = PlayerConstants::max_targets;

    bool reached_max_targets = selections.size() + (targets ? targets->size() : 0) == max_targets;

    static std::vector<std::pair<std::string, bool>> columns{
        {"CN",          true },
        {"Player name", true },
        {"HP",          true },
        {"AP",          true },
        {"K",           true },
        {"D",           true },
        {"Weapon",      true },
        {"Clip",        true },
        {"Magazine",    true },
        {"Position",    true },
        {"Viewangles",  false}
    };

    /* Lambda for checking if the specified column is enabled. */
    static constexpr auto is = [](const std::string &str) -> bool {
        for (const auto &c : columns) {
            if (c.first == str) {
                return c.second;
            }
        }

        return false;
    };

    ImGui::Begin("##player_list", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
        /* Create checkboxes for toggling table columns. */
        ImGui::BeginGroup();
            for (auto &c : columns) {
                if ("CN" == c.first) {
                    ImGui::Checkbox("CN (Client Number)", &c.second);
                } else if ("HP" == c.first) {
                    ImGui::Checkbox("HP (Health)", &c.second);
                } else if ("AP" == c.first) {
                    ImGui::Checkbox("AP (Armour)", &c.second);
                } else if ("K" == c.first) {
                    ImGui::Checkbox("K (Kills)", &c.second);
                } else if ("D" == c.first) {
                    ImGui::Checkbox("D (Deaths)", &c.second);
                } else {
                    ImGui::Checkbox(c.first.c_str(), &c.second);
                }
            }
        ImGui::EndGroup();
        ImGui::SameLine();
        ImGui::VerticalSpacing(5.0f);

        /* Count the amount of enabled columns. */
        int columns_count = !!in_selection * 1;
        for (const auto &c : columns) {
            if (c.second) {
                ++columns_count;
            }
        }
        
        ImGui::PushStyleColor(ImGuiCol_TableBorderLight, MenuConfig::table_border_color);
        ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, MenuConfig::table_border_color);
        ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, MenuConfig::table_header_bg_color);

        /* Create table containing player information. */
        ImGui::BeginTable("##players", columns_count,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollFreezeTopRow |
                          ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersVFullHeight |
                          ImGuiTableFlags_NoHostExtendY,
                          {0.0f, ImGui::GetFrameHeight() * (MenuConfig::player_info_rows + 0.4f)});
            /* Set up columns. */
            if (in_selection) {
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 23.0f * scale.x);
            }

            for (const auto &c : columns) {
                if (c.second) {
                    ImGui::TableSetupColumn(c.first.c_str(),
                                            ImGuiTableColumnFlags_WidthAlwaysAutoResize);
                }
            }

            /* Set up column headers. */
            const float row_height = ImGui::GetTextLineHeight() +
                ImGui::GetStyle().CellPadding.y * 2.0f;
            ImGui::TableNextRow(ImGuiTableRowFlags_Headers, row_height);
            for (unsigned i = 0; columns_count != i; ++i) {
                if (!ImGui::TableSetColumnIndex(i)) {
                    continue;
                }

                /* Add a checkmark, which we will use for multi-target
                 * selection, in the first column instead of a label. */
                if (0 == i && targets) {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0.0f, 0.0f});
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, MenuConfig::table_header_chkmrk_color);

                    bool b = plist.size() == selections.size();
                    if (ImGui::Checkbox("##", &b) && !reached_max_targets) {
                        /* We always clear the list, independent of whether we
                         * intend to deselect all or select all players. */
                        selections.clear();

                        if (b) {
                            /* Select all eligible players. */
                            for (const auto &p : plist) {
                                if (max_targets == selections.size() + targets->size()) {
                                    reached_max_targets = true;
                                    break;
                                }

                                /* Find (if any) a selected player with an
                                 * identical name. */
                                auto sel_pos = selections.cend();
                                for (auto it = selections.cbegin(); selections.cend() != it;
                                     ++it) {
                                    /* Comparing names for reasons described in
                                     * comment above NOTE [1]. */
                                    if (p->get_name() == (*it)->get_name()) {
                                        sel_pos = it;
                                        break;
                                    }
                                }

                                /* We prevent twins (players with identical
                                 * names) or dupes (identical players (hence
                                 * identical data)) from being selected as
                                 * we've yet to come up with a (semi-)decent
                                 * way of handling different players with
                                 * identical names as client numbers do not
                                 * carry-over across sessions nor is there any
                                 * other uniquely-identifiable information we
                                 * could associate with a player.
                                 *
                                 * TODO: change the logic once a reliable
                                 *       method of handling players with
                                 *       identical names has been developed. */
                                if (targets->cend() != TargetManager::find(p->get_name()) ||
                                        selections.cend() != sel_pos) {
                                    continue;
                                }

                                selections.push_back(p);
                            }
                        }
                    }

                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar();
                } else {
                    ImGui::TableHeader(ImGui::TableGetColumnName(i));
                }
            }

            /* Populate columns. */
            for (std::vector<Target>::size_type i = 0; plist.size() != i; ++i) {
                const auto &p = plist[i];

                ImGui::TableNextRow();
                    /* If we intend to have selectable rows. */
                    if (in_selection && targets) {
                        /* Check whether the current player is already a target
                         * or not. */
                        const bool is_target = TargetManager::find(p->get_name()) !=
                            targets->cend();
                        bool row_selected = is_target;

                        /* Check whether the current player is selected or not. */
                        auto pos = selections.cend();
                        for (auto it = selections.cbegin(); selections.cend() != it; ++it) {
                            /* We need to check the client number instead of
                             * simply comparing ``p`` to ``*it`` because we
                             * currently repopulate the player list on each
                             * frame, hence all ``p``s and ``*it``s point to
                             * different memory addresses.
                             *
                             * NOTE [1]: even if we opt to repopulate the
                             *           player list as documented in
                             *           ``PlayerList::update``, this should
                             *           stay the same as whenever the list
                             *           does get cleared and renewed, ``*it``
                             *           and ``p`` will never be equal (see
                             *           reasoning above).
                             *
                             * NOTE [2]: we could use the name instead of
                             *           client number to compare for equality
                             *           as it is guaranteed to be reliable by
                             *           twin prevention (we do not allow
                             *           selecting players with identical
                             *           names) but doing so would mark the
                             *           twin as selected which might be seen
                             *           as misleading, so choosing client
                             *           number comparisons is a better choice
                             *           for now. */
                            if (p->get_cn() == (*it)->get_cn()) {
                                pos = it;
                                row_selected = true;
                                break;
                            }
                        }

                        /* Check if the current player is a twin of a (at some
                         * point) previously selected player. */
                        bool is_twin = false;
                        for (const auto &s : selections) {
                            /* Make sure the client numbers do not match so as
                             * to prevent a player being a twin of themselves. */
                            if (s->get_name() == p->get_name() && s->get_cn() != p->get_cn()) {
                                is_twin = true;
                                break;
                            }
                        }

                        /* Slightly correct the Y value of the cursor position to
                         * align the ``ImGui::Selectable`` properly. */
                        ImGui::GetCurrentWindow()->DC.CursorPos.y += 0.9f;

                        /* Add a selectable over the whole row so we're not limited
                         * to checkbox-selection only. */
                        ImGui::PushID(i);
                            ImGui::PushStyleColor(ImGuiCol_Header, is_twin ?
                                                    MenuConfig::table_row_twin_color :
                                                    reached_max_targets && !row_selected ?
                                                        MenuConfig::table_row_max_targets_color :
                                                        MenuConfig::table_row_color);
                            ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
                                                  MenuConfig::table_row_hover_color);
                            ImGui::PushStyleColor(ImGuiCol_HeaderActive,
                                                  MenuConfig::table_row_active_color);

                            /* Disable the row if the player is already a
                             * target or has an identical name to one, prevent
                             * selecting a player with an identical name, and
                             * do not allow passing the ``max_targets`` mark. */
                            if (is_target || is_twin || (reached_max_targets && !row_selected)) {
                                ImGui::BeginDisabled(1.0f);
                            }

                            if (ImGui::Selectable("##", row_selected ||
                                                  is_twin || reached_max_targets,
                                                  ImGuiSelectableFlags_SpanAllColumns)) {
                                if (row_selected) { selections.erase(pos); }
                                else { selections.push_back(p); }
                            }

                            if (is_target) {
                                ImGui::EndDisabled();

                                if (ImGui::IsItemHovered()) {
                                    ImGui::ShowTooltip("Player is already a target or has an "
                                                       "identical name to one. We currently do not "
                                                       "support multiple targets with identical "
                                                       "names due to complications, which may "
                                                       "possibly be resolved at some point. That "
                                                       "being said, to remove the target, click "
                                                       "\"Cancel\", select them in the main "
                                                       "windows' target selection panel and click "
                                                       "\"Remove selected (...)\".",
                                                       MenuConfig::tooltip_padding);
                                }
                            } else if (is_twin) {
                                ImGui::EndDisabled();

                                if (ImGui::IsItemHovered()) {
                                    ImGui::ShowTooltip("A previously selected player has an "
                                                       "identical name. We currently do not "
                                                       "support multiple targets with identical "
                                                       "names due to complications, which may "
                                                       "possibly be resolved at some point.",
                                                       MenuConfig::tooltip_padding);
                                }
                            } else if (reached_max_targets && !row_selected) {
                                ImGui::EndDisabled();

                                if (ImGui::IsItemHovered()) {
                                    ImGui::ShowTooltip(("Maximum amount (" +
                                                        std::to_string(max_targets) +
                                                        ") of targets reached.").c_str(),
                                                       MenuConfig::tooltip_padding);
                                }
                            }

                            ImGui::PopStyleColor(3);
                        ImGui::PopID();
                        ImGui::SameLine();

                        ImGui::TableSetColumnIndex(0);
                            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0.0f, 0.0f});
                            ImGui::PushID(i);
                                if (ImGui::Checkbox("##", &row_selected)) {
                                    if (row_selected) { selections.push_back(p); }
                                    else { selections.erase(pos); }
                                }
                            ImGui::PopID();
                            ImGui::PopStyleVar();
                    }

                    /* FIXME: this is most likely unbelievably inefficient
                     * (performance-and-clarity-wise). */
                    unsigned idx = !!in_selection * 1;
                    if (is("CN")) {
                        ImGui::TableSetColumnIndex(idx++);
                            ImGui::Text("%d", p->get_cn());
                    }

                    if (is("Player name")) {
                        ImGui::TableSetColumnIndex(idx++);
                            ImGui::Text(p->get_name().c_str());
                    }

                    if (is("HP")) {
                        ImGui::TableSetColumnIndex(idx++);
                            ImGui::Text("%d", p->get_health());
                    }

                    if (is("AP")) {
                        ImGui::TableSetColumnIndex(idx++);
                            ImGui::Text("%d", p->get_armour());
                    }

                    if (is("K")) {
                        ImGui::TableSetColumnIndex(idx++);
                            ImGui::Text("%d", p->get_kills());
                    }

                    if (is("D")) {
                        ImGui::TableSetColumnIndex(idx++);
                            ImGui::Text("%d", p->get_deaths());
                    }

                    if (is("Weapon")) {
                        ImGui::TableSetColumnIndex(idx++);
                            ImGui::Text(p->get_current_weapon()->get_name().c_str());
                    }

                    if (is("Clip")) {
                        ImGui::TableSetColumnIndex(idx++);
                            ImGui::Text("%d", p->get_current_weapon()->get_clip());
                    }

                    if (is("Magazine")) {
                        ImGui::TableSetColumnIndex(idx++);
                            ImGui::Text("%d", p->get_current_weapon()->get_mag());
                    }

                    if (is("Position")) {
                        ImGui::TableSetColumnIndex(idx++);
                            const auto pos = p->get_position();
                            ImGui::Text("(%.1f; %.1f; %.1f)", pos[0], pos[1], pos[2]);
                    }

                    if (is("Viewangles")) {
                        ImGui::TableSetColumnIndex(idx++);
                            const auto angles = p->get_viewangles();
                            ImGui::Text("(%.1f; %.1f)", angles[0], angles[1]);
                    }
            }
        ImGui::EndTable();

        ImGui::PopStyleColor(3);

        if (in_selection) {
            if (0 == selections.size()) {
                ImGui::BeginDisabled();
                ImGui::Button("Save", button_size);
                ImGui::EndDisabled();

                if (ImGui::IsItemHovered()) {
                    ImGui::ShowTooltip("At least one (1) player needs to be selected.",
                                       MenuConfig::tooltip_padding);
                }
            } else {
                if (ImGui::Button("Save", button_size)) {
                    for (auto p : selections) {
                        targets->push_back({p->get_name(), p});
                    }

                    selections.clear();
                    *in_selection = false;
                }

                if (ImGui::IsItemHovered()) {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel", button_size)) {
                selections.clear();
                *in_selection = false;
            }

            if (ImGui::IsItemHovered()) {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            }
        }
    ImGui::End();

    ImGui::PopStyleVar();
}

void Menu::generate_categories_sub()
{
    const float category_width = scale.x * MenuConfig::category_base_size.x;
    const float category_height = scale.y * MenuConfig::category_base_size.y;

    ImGui::SetColumnWidth(-1, category_width);

    ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, {0.5f, 0.5f});

    ImGui::PushStyleColor(ImGuiCol_Header, MenuConfig::category_color);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, MenuConfig::category_hovered_color);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, MenuConfig::category_active_color);

    const auto &categories = MenuConfig::categories;
    constexpr auto &fillers = MenuConfig::fillers;

    /* Distribute unused height across fillers (if there are any) uniformly. */
    const float fh = (0 < fillers.size()) *
        (size.y - category_height * categories.size()) / fillers.size();

    for (decltype(MenuConfig::categories)::size_type i = 0, f_i = 0;
         categories.size() != i; ++i) {
        if (ImGui::Selectable(Utils::String::to_upper(categories[i]).c_str(),
                              selected_index == i, 0,
                              {category_width, category_height})) {
            selected_index = i;
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }

        /* Check if we're on a filler index. */
        if (f_i < fillers.size() && fillers[f_i] == i) {
            ImGui::Dummy({0.0f, fh});
            ++f_i;
        }
    }

    ImGui::PopStyleColor(3);

    ImGui::PopStyleVar();
}

void Menu::generate_aimbot_sub()
{
    const auto settings = &AimbotConfig::settings;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, MenuConfig::panel_padding);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {2.0f, 2.0f});
    ImGui::BeginPanel("Toggles");
        if (ImGui::CustomCheckbox("Enabled", &Aimbot::enabled)) {
            Weapon::toggle_auto_attack_all(IS_SET(*settings, AimbotConfig::auto_attack));
        }

        if (!Aimbot::enabled) {
            ImGui::BeginDisabled();
        }
        
        ImGui::CustomCheckboxFlags("Lock on target", settings, AimbotConfig::lock_on_target);
        help_marker("Maintain target until they are dead or out of the FOE.");

        ImGui::CustomCheckboxFlags("Auto-aim", settings, AimbotConfig::auto_aim);
        help_marker("Do not wait for user input to aim - always scan for new targets in the FOE.");

        if (ImGui::CustomCheckboxFlags("Auto-attack", settings, AimbotConfig::auto_attack)) {
            Weapon::toggle_auto_attack_all(IS_SET(*settings, AimbotConfig::auto_attack));
        }
        help_marker("Automatically attack the target. (If equipped with a pistol or knife, "
                    "disregards the imposed breaks in-between shots/slashes; if equipped with a "
                    "grenade, does nothing).");

        ImGui::CustomCheckboxFlags("Aim through walls", settings, AimbotConfig::aim_through_walls);

        ImGui::CustomCheckboxFlags("Aim at teammates", settings, AimbotConfig::aim_at_teammates);
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetStyle().IndentSpacing + 5.0f);
        ImGui::Indent();
            if (!IS_SET(*settings, AimbotConfig::aim_at_teammates)) { ImGui::BeginDisabled(); }
            ImGui::CustomCheckboxFlags("Hide \"Blocked\" icon",
                                       settings, AimbotConfig::hide_blocked);
            help_marker("Hide the crossed circle (\"Blocked\" icon) when aiming at a teammate.");
            if (!IS_SET(*settings, AimbotConfig::aim_at_teammates)) { ImGui::EndDisabled(); }
        ImGui::Unindent();
        ImGui::PopStyleVar();

        ImGui::CustomCheckboxFlags("Disable mouse", settings, AimbotConfig::disable_mouse);
        help_marker("Disable mouse when the aimbot is active (a target is in the FOE).");

        ImGui::CustomCheckboxFlags("Disable on reload", settings, AimbotConfig::disable_on_reload);
        help_marker("Disable aimbot on weapon reload.");

        ImGui::CustomCheckboxFlags("Smooth aiming", settings, AimbotConfig::smoothing_enabled);
        help_marker("Prevent immediate snapping to target by instead moving the players' "
                    "viewangles in steps, whose size is determined by the \"Smoothing\" "
                    "variable.");

        if (ImGui::CustomCheckboxFlags("Prioritize targets", &AimbotConfig::target_settings,
                                       AimbotConfig::prioritize_targets)) {
            UNSET(AimbotConfig::target_settings, AimbotConfig::aim_at_targets_only);
        }
        help_marker("Prioritize targets over regular players. Meaning that if a player and a "
                    "target are both on screen, the aimbot will first aim at the target.");

        if (ImGui::CustomCheckboxFlags("Aim only at targets", &AimbotConfig::target_settings,
                                       AimbotConfig::aim_at_targets_only)) {
            UNSET(AimbotConfig::target_settings, AimbotConfig::prioritize_targets);
        }
        help_marker("Aim only at targets - ignore all other players.");

        if (!Aimbot::enabled) {
            ImGui::EndDisabled();
        }
    ImGui::EndPanel();
    ImGui::PopStyleVar();

    ImGui::VerticalSpacing(MenuConfig::window_padding.x);

    if (!Aimbot::enabled) { ImGui::BeginDisabled(); }
    ImGui::BeginPanel("Tweakables", "(?)", false, -MenuConfig::window_padding.x * 4.25f);
        ImGui::SliderFloat("FOE", &AimbotConfig::foe, 0.1f, 360.0f, "%.1f deg");
        help_marker("Field of Effect - amount of degrees to scan around the player for targets. "
                    "[MIN: 0.1 deg; MAX: 360.0 deg]");
        AimbotConfig::foe = 360.0f < AimbotConfig::foe ? 360.0f :
                            0.1f > AimbotConfig::foe ? 0.1f : AimbotConfig::foe;

        if (!(IS_SET(*settings, AimbotConfig::smoothing_enabled))) { ImGui::BeginDisabled(); }
        ImGui::SliderFloat("Smoothing", &AimbotConfig::smoothing, 1.1f, 100.0f, "%.1f");
        if (!(IS_SET(*settings, AimbotConfig::smoothing_enabled))) { ImGui::EndDisabled(); }
        help_marker(std::string(!(IS_SET(*settings, AimbotConfig::smoothing_enabled)) ?
                                "DISABLED: \"Smooth aiming\" must be enabled.\n" : "") +
                    "Amount of times to decrease the size of a step taken when aiming at a "
                    "target. [MIN: 1.1; MAX: 100.0]");
        AimbotConfig::smoothing = 100.0f < AimbotConfig::smoothing ? 100.0f :
                                     1.1f > AimbotConfig::smoothing ?
                                     1.1f : AimbotConfig::smoothing;
    ImGui::EndPanel();

    ImGui::GetCurrentWindow()->DC.CursorPos = {
        ImGui::GetItemRectMin().x,
        ImGui::GetItemRectMax().y + MenuConfig::window_padding.y * 3.25f
    };

    generate_target_sel_panel(ImGui::GetItemRectMax().x - ImGui::GetItemRectMin().x);
    if (!Aimbot::enabled) { ImGui::EndDisabled(); }

    ImGui::PopStyleVar();
}

void Menu::generate_esp_sub()
{
    auto settings = &ESPConfig::player_esp_settings;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, MenuConfig::panel_padding);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {2.0f, 2.0f});
    ImGui::BeginPanel("Player ESP");
        ImGui::CustomCheckbox("Enabled##player_esp", &ESP::player_esp_enabled);

        BEGIN_DISABLED(!ESP::player_esp_enabled);
        if (ImGui::CustomCheckboxFlags("Show enemies", settings, ESPConfig::show_enemies)) {
            if (!IS_SET(*settings, ESPConfig::show_teammates)) {
                SET(*settings, ESPConfig::show_enemies);
            }
        }

        if (ImGui::CustomCheckboxFlags("Show teammates", settings, ESPConfig::show_teammates)) {
            if (!IS_SET(*settings, ESPConfig::show_enemies)) {
                SET(*settings, ESPConfig::show_teammates);
            }
        }

        ImGui::CustomCheckboxFlags("Show name", settings, ESPConfig::show_player_name);
        ImGui::CustomCheckboxFlags("Show client number", settings, ESPConfig::show_client_number);
        ImGui::CustomCheckboxFlags("Show distance##player", settings, ESPConfig::show_distance);
        ImGui::CustomCheckboxFlags("Show HP bar", settings, ESPConfig::show_hp_bar);
        ImGui::CustomCheckboxFlags("Show AP bar", settings, ESPConfig::show_ap_bar);
        ImGui::CustomCheckboxFlags("Show reload bar", settings, ESPConfig::show_reload_bar);
        help_marker("Displays a bar to the left of the ESP box which indicates the time left "
                    "until the player fully reloads.");

        /* Generate basic information target selection. */
        ImGui::Text("Show for");
        ImGui::SameLine();

        std::string preview = IS_SET(*settings, ESPConfig::show_info_all) ?
                                  "everyone" :
                                  IS_SET(*settings, ESPConfig::show_info_targets) ?
                                      "targets" : "current target";

        /* FIXME: assign 125.0f to a variable. */
        ImGui::SetNextItemWidth(125.0f);
        if (ImGui::BeginCombo("##show_basic_for", preview.c_str())) {
            /* Add top and left side padding. */
            ImGui::Dummy({-1.0f, MenuConfig::panel_padding.y / 2});
            ImGui::Dummy({MenuConfig::panel_padding.x / 4, 0.0f});

            ImGui::SameLine();
            ImGui::BeginGroup();
            if (ImGui::Selectable("everyone", IS_SET(*settings, ESPConfig::show_info_all), 0,
                                  {125.0f - 2.0f, 0.0f})) {
                SET(*settings, ESPConfig::show_info_all);
                UNSET(*settings, ESPConfig::show_info_targets);
                UNSET(*settings, ESPConfig::show_info_target);
            }

            if (ImGui::Selectable("targets", IS_SET(*settings, ESPConfig::show_info_targets),
                                  0, {125.0f - 2.0f, 0.0f})) {
                SET(*settings, ESPConfig::show_info_targets);
                UNSET(*settings, ESPConfig::show_info_all);
                UNSET(*settings, ESPConfig::show_info_target);
            }

            if (ImGui::Selectable("current target",
                                  IS_SET(*settings, ESPConfig::show_info_target), 0,
                                  {125.0f - 2.0f, 0.0f})) {
                SET(*settings, ESPConfig::show_info_target);
                UNSET(*settings, ESPConfig::show_info_targets);
                UNSET(*settings, ESPConfig::show_info_all);
            }
            ImGui::EndGroup();
            ImGui::SameLine();

            /* Add right and bottom side padding. */
            ImGui::Dummy({MenuConfig::panel_padding.x / 4, 0.0f});
            ImGui::Dummy({-1.0f, MenuConfig::panel_padding.y / 2});
            ImGui::EndCombo();
        }
        help_marker("Display basic ESP (player name, health bar, ESP box) for:\n\t"
                    "- everyone: all players as defined per \"Show enemies\" and \"Show "
                    "teammates\";\n\t"
                    "- targets: all targets;\n\t"
                    "- current target: current triggerbot/aimbot target (available only if either "
                    "of said features is enabled).");

        ImGui::CustomCheckboxFlags("Show health", settings, ESPConfig::show_add_health);
        ImGui::CustomCheckboxFlags("Show armour", settings, ESPConfig::show_add_armour);
        ImGui::CustomCheckboxFlags("Show kills", settings, ESPConfig::show_add_kills);
        ImGui::CustomCheckboxFlags("Show deaths", settings, ESPConfig::show_add_deaths);
        ImGui::CustomCheckboxFlags("Show weapon", settings, ESPConfig::show_add_weapon);
        ImGui::CustomCheckboxFlags("Show weapon ammo", settings, ESPConfig::show_add_ammo);

        /* Generate additional information target selection. */
        ImGui::Text("Show info. for");
        ImGui::SameLine();

        preview = IS_SET(*settings, ESPConfig::show_add_info_all) ?
                      "everyone" :
                      IS_SET(*settings, ESPConfig::show_add_info_targets) ?
                          "targets" : "current target";

        /* FIXME: assign 125.0f to a variable. */
        ImGui::SetNextItemWidth(125.0f);
        if (ImGui::BeginCombo("##show_add_for", preview.c_str())) {
            /* Add top and left side padding. */
            ImGui::Dummy({-1.0f, MenuConfig::panel_padding.y / 2});
            ImGui::Dummy({MenuConfig::panel_padding.x / 4, 0.0f});

            ImGui::SameLine();
            ImGui::BeginGroup();
                if (ImGui::Selectable("everyone", IS_SET(*settings, ESPConfig::show_add_info_all),
                                      0, {125.0f - 2.0f, 0.0f})) {
                    SET(*settings, ESPConfig::show_add_info_all);
                    UNSET(*settings, ESPConfig::show_add_info_targets);
                    UNSET(*settings, ESPConfig::show_add_info_target);
                }

                if (ImGui::Selectable("targets", IS_SET(*settings, ESPConfig::show_add_info_targets),
                                      0, {125.0f - 2.0f, 0.0f})) {
                    SET(*settings, ESPConfig::show_add_info_targets);
                    UNSET(*settings, ESPConfig::show_add_info_all);
                    UNSET(*settings, ESPConfig::show_add_info_target);
                }

                if (ImGui::Selectable("current target",
                                      IS_SET(*settings, ESPConfig::show_add_info_target), 0,
                                      {125.0f - 2.0f, 0.0f})) {
                    SET(*settings, ESPConfig::show_add_info_target);
                    UNSET(*settings, ESPConfig::show_add_info_targets);
                    UNSET(*settings, ESPConfig::show_add_info_all);
                }
            ImGui::EndGroup();
            ImGui::SameLine();

            /* Add right and bottom side padding. */
            ImGui::Dummy({MenuConfig::panel_padding.x / 4, 0.0f});
            ImGui::Dummy({-1.0f, MenuConfig::panel_padding.y / 2});
            ImGui::EndCombo();
        }
        help_marker("Show window with additional information for:\n\t"
                    "- everyone: all players as defined per \"Show enemies\" and \"Show "
                    "teammates\";\n\t"
                    "- targets: all targets;\n\t"
                    "- current target: current triggerbot/aimbot target (active only if either "
                    "of said features is enabled).");
        END_DISABLED(!ESP::player_esp_enabled);
    ImGui::EndPanel();
    const ImRect pos{ImGui::GetItemRectMin(), ImGui::GetItemRectMax()};

    ImGui::VerticalSpacing(MenuConfig::window_padding.x);

    ImGui::BeginPanel("Tweakables", "(?)", false, -MenuConfig::window_padding.x * 4.25f);
        ImGui::SliderFloat("Info. distance", &ESPConfig::max_distance, 10.0f, 100.0f, "%.1f u");
        help_marker("The maximum distance a player/entity can be from the local player for the "
                    "additional information window to still be displayed. "
                    "[MIN: 10.0 units; MAX: 100.0 units]");
    ImGui::EndPanel();

    ImGui::GetCurrentWindow()->DC.CursorPos = {
        ImGui::GetItemRectMin().x,
        ImGui::GetItemRectMax().y + MenuConfig::window_padding.y * 3.25f
    };

    settings = &ESPConfig::entity_esp_settings;
    ImGui::BeginPanel("Entity ESP", "(?)");
        ImGui::CustomCheckbox("Enabled##entity_esp", &ESP::entity_esp_enabled);

        BEGIN_DISABLED(!ESP::entity_esp_enabled);
        if (ImGui::BeginCombo("Select static entities", nullptr, ImGuiComboFlags_NoPreview)) {
            /* Add top and left side padding. */
            ImGui::Dummy({-1.0f, MenuConfig::panel_padding.y / 2});
            ImGui::Dummy({MenuConfig::panel_padding.x / 4, 0.0f});

            ImGui::SameLine();
            ImGui::BeginGroup();
                ImGui::CheckboxFlags("Light", settings, ESPConfig::show_light);
                ImGui::CheckboxFlags("Playerstart", settings, ESPConfig::show_playerstart);
                ImGui::CheckboxFlags("Pistol magazines", settings, ESPConfig::show_pistol);
                ImGui::CheckboxFlags("Rifle ammo box", settings, ESPConfig::show_ammobox);
                ImGui::CheckboxFlags("Grenades", settings, ESPConfig::show_grenade);
                ImGui::CheckboxFlags("Health packs", settings, ESPConfig::show_health);
                ImGui::CheckboxFlags("Helmets", settings, ESPConfig::show_helmet);
                ImGui::CheckboxFlags("Kevlar", settings, ESPConfig::show_kevlar);
                ImGui::CheckboxFlags("Akimbo (dual pistols)",
                                        settings, ESPConfig::show_akimbo);
                ImGui::CheckboxFlags("Mapmodels", settings, ESPConfig::show_mapmodel);
                ImGui::CheckboxFlags("Triggers", settings, ESPConfig::show_trigger);
                ImGui::CheckboxFlags("Ladders", settings, ESPConfig::show_ladder);
                ImGui::CheckboxFlags("CTF flags", settings, ESPConfig::show_ctf_flag);
                ImGui::CheckboxFlags("Sound", settings, ESPConfig::show_sound);
                ImGui::CheckboxFlags("Clips", settings, ESPConfig::show_clip);
                ImGui::CheckboxFlags("Player clips", settings, ESPConfig::show_pclip);
            ImGui::EndGroup();
            ImGui::SameLine();

            /* Add right and bottom side padding. */
            ImGui::Dummy({MenuConfig::panel_padding.x / 4, 0.0f});
            ImGui::Dummy({-1.0f, MenuConfig::panel_padding.y / 2});
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("Select dynamic entities", nullptr, ImGuiComboFlags_NoPreview)) {
            /* Add top and left side padding. */
            ImGui::Dummy({-1.0f, MenuConfig::panel_padding.y / 2});
            ImGui::Dummy({MenuConfig::panel_padding.x / 4, 0.0f});

            ImGui::SameLine();
            ImGui::BeginGroup();
                ImGui::CheckboxFlags("Grenades", settings, ESPConfig::show_dyn_grenade);
                ImGui::CheckboxFlags("Gibs", settings, ESPConfig::show_gibs);
            ImGui::EndGroup();
            ImGui::SameLine();

            /* Add right and bottom side padding. */
            ImGui::Dummy({MenuConfig::panel_padding.x / 4, 0.0f});
            ImGui::Dummy({-1.0f, MenuConfig::panel_padding.y / 2});
            ImGui::EndCombo();
        }

        ImGui::CustomCheckboxFlags("Show hidden entities", settings, ESPConfig::show_hidden);
        ImGui::CustomCheckboxFlags("Show names", settings, ESPConfig::show_ent_name);
        ImGui::CustomCheckboxFlags("Show IDs", settings, ESPConfig::show_ent_id);
        ImGui::CustomCheckboxFlags("Show distance##ent", settings, ESPConfig::show_ent_distance);
        ImGui::CustomCheckboxFlags("Align ESP box with model", settings, ESPConfig::align_box);
        help_marker("Entities for which this applies include the following:"
                    "\n\t- pistol;\n\t- ammobox;\n\t- grenade;\n\t- health;"
                    "\n\t- helmet;\n\t- kevlar;\n\t- akimbo.");
        ImGui::CustomCheckboxFlags("Show time until despawn",
                                   settings, ESPConfig::show_despawn_time);
        help_marker("Display time left until a dynamic entity despawns.");
        END_DISABLED(!ESP::entity_esp_enabled);
    ImGui::EndPanel();

    ImGui::GetCurrentWindow()->DC.CursorPos = {
        pos.Min.x,
        pos.Max.y + MenuConfig::window_padding.y * 3.25f
    };

    BEGIN_DISABLED(!ESP::player_esp_enabled);
    generate_target_sel_panel(pos.Max.x - pos.Min.x);
    END_DISABLED(!ESP::player_esp_enabled);

    ImGui::PopStyleVar(2);
}

void Menu::generate_triggerbot_sub()
{

}

void Menu::generate_misc_sub()
{
    auto &settings = MiscConfig::settings;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, MenuConfig::panel_padding);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {2.0f, 2.0f});

    ImGui::BeginPanel("Fullbright");
        if (ImGui::CustomCheckbox("Enabled##fullbright", &Misc::fullbright)) {
            Game::fullbright(Misc::fullbright, MiscConfig::fullbright_col);
        }

        BEGIN_DISABLED(!Misc::fullbright);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {10.0f, 10.0f});
            if (ImGui::ColorEdit3("Color", &MiscConfig::fullbright_col[0],
                                  ImGuiColorEditFlags_NoInputs)) {
                Game::fullbright(true, MiscConfig::fullbright_col, false);
            }
            ImGui::PopStyleVar();
        END_DISABLED(!Misc::fullbright);
    ImGui::EndPanel();

    ImGui::VerticalSpacing(MenuConfig::window_padding.x);

    ImGui::BeginPanel("Recoil, spread, knockback", "", false, 200.0f);
        if (ImGui::CustomCheckbox("Enabled##rsk", &Misc::rsk)) {
            if (!Misc::rsk) {
                /* By disabling incrementation of consecutive shots we are able to
                 * directly affect the spread of the weapon, which would otherwise
                 * be constant (excluding the case when consecutive_shots == 1)
                 * (see sub_463D70 at 0x463D70, where we can see that if
                 * consecutive_shots > 2, the spread is defined as 0x37 (55),
                 * otherwise, if consecutive_shots <= 0, the spread is defined per
                 * the spread variable in the weapon_info structure). */
                Weapon::toggle_consec_shots(true);
                Weapon::reset_rsk();
            } else {
                Weapon::toggle_consec_shots(false);
            }
        }
        
        auto &wpn = *LocalPlayerConfig::ptr->get_current_weapon();
        static constexpr std::int16_t min = -32767;
        static constexpr std::int16_t max = 32767;

        BEGIN_DISABLED(!Misc::rsk);
        ImGui::SliderScalar("Recoil", ImGuiDataType_S16, &wpn.get_recoil(), &min, &max);
        ImGui::SliderScalar("Spread", ImGuiDataType_S16, &wpn.get_spread(), &min, &max);
        ImGui::SliderScalar("Knockback", ImGuiDataType_S16, &wpn.get_knockback(), &min, &max);
        help_marker("WARNING: will be detected as teleport/speed hack (and will thus lead to a "
                    "kick/ban) if used in high frequency with high values.");

        if (ImGui::Button("Apply to all weapons", {200.0f, 0.0f})) {
            const auto wpns = reinterpret_cast<Weapon **>(LocalPlayerConfig::weapon_list_address);
            for (std::size_t i = 0; WeaponConstants::weapon_count != i; ++i) {
                wpns[i]->set_recoil(wpn.get_recoil());
                wpns[i]->set_spread(wpn.get_spread());
                wpns[i]->set_knockback(wpn.get_knockback());
            }
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        END_DISABLED(!Misc::rsk);
    ImGui::EndPanel();

    ImGui::VerticalSpacing(MenuConfig::window_padding.x);

    ImGui::BeginPanel("Flying", "", false, 160.0f);
        if (ImGui::CustomCheckbox("Enabled##flying", &Misc::flying)) {
            Misc::toggle_flying(Misc::flying, MiscConfig::flying_mode);
        }
        help_marker("WARNING: high speed flying will trigger an automatic kick/ban for "
                    "teleport/speed hack.");

        BEGIN_DISABLED(!Misc::flying);
        ImGui::Text("Mode");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(125.0f);
        if (ImGui::BeginCombo("##flying_mode",
                              MiscConfig::fly_mode_names[MiscConfig::flying_mode].c_str())) {
            /* Add top and left side padding. */
            ImGui::Dummy({-1.0f, MenuConfig::panel_padding.y / 2});
            ImGui::Dummy({MenuConfig::panel_padding.x / 4, 0.0f});

            ImGui::SameLine();
            ImGui::BeginGroup();
            if (ImGui::Selectable("ghost", MiscConfig::ghost == MiscConfig::flying_mode,
                                  0, {125.0f - 2.0f, 0.0f})) {
                Misc::toggle_flying(Misc::flying,
                                    MiscConfig::flying_mode = MiscConfig::ghost);
            }

            if (ImGui::Selectable("inverse Minecraft",
                                  MiscConfig::inverse_minecraft == MiscConfig::flying_mode,
                                  0, {125.0f - 2.0f, 0.0f})) {
                Misc::toggle_flying(Misc::flying,
                                    MiscConfig::flying_mode = MiscConfig::inverse_minecraft);
            }

            if (ImGui::Selectable("water", MiscConfig::water == MiscConfig::flying_mode,
                                  0, {125.0f - 2.0f, 0.0f})) {
                Misc::toggle_flying(Misc::flying,
                                    MiscConfig::flying_mode = MiscConfig::water);
            }

            if (ImGui::Selectable("noclip", MiscConfig::noclip == MiscConfig::flying_mode,
                                  0, {125.0f - 2.0f, 0.0f})) {
                Misc::toggle_flying(Misc::flying,
                                    MiscConfig::flying_mode = MiscConfig::noclip);
            }

            if (ImGui::IsItemHovered()) {
                ImGui::ShowTooltip("WARNING: will trigger an automatic ban within seconds of "
                                   "flying through the bounding box of the map.",
                                   MenuConfig::tooltip_padding);
            }
            ImGui::EndGroup();
            ImGui::SameLine();

            /* Add right and bottom side padding. */
            ImGui::Dummy({MenuConfig::panel_padding.x / 4, 0.0f});
            ImGui::Dummy({-1.0f, MenuConfig::panel_padding.y / 2});
            ImGui::EndCombo();
        }

        auto speed = reinterpret_cast<float *>(Config::mod_base_address +
                                               Offsets::Module::fly_speed);
        ImGui::SliderFloat("Speed", speed, 0.0f, 100.0f, "%.1f");
        END_DISABLED(!Misc::flying);
    ImGui::EndPanel();

    ImGui::GetCurrentWindow()->DC.CursorPos = {
        ImGui::GetItemRectMin().x,
        ImGui::GetItemRectMax().y + MenuConfig::window_padding.y * 3.25f
    };

    ImGui::BeginPanel("Visuals");
        if (ImGui::CustomCheckbox("Wallhack", &Misc::wallhack)) {
            Misc::toggle_wallhack(Misc::wallhack);
        }
    ImGui::EndPanel();

    ImGui::PopStyleVar(2);
}

void Menu::generate_styling_sub()
{

}

void Menu::generate_load_save_sub()
{

}

void Menu::generate_target_sel_panel(const float width,
                                     const unsigned rows)
{
    ImGui::BeginPanel("Targets", "", false, -MenuConfig::window_padding.x);
        /* Position buttons and update the panels' width manually since
         * (as of 11/07/2020) tables _seem_ to disregard grouping. */
        ImGui::Dummy({width - 2 * MenuConfig::window_padding.x, 0.0f});
        const auto button_x = ImGui::GetCurrentWindow()->DC.CursorPos.x;

        /* Move the cursor up a bit to match the specified padding. */
        ImGui::GetCurrentWindow()->DC.CursorPos.y -= MenuConfig::panel_padding.y / 2;

        auto &targets = TargetManager::get_all();
        static std::vector<std::vector<Target>::size_type> selections;
        ImGui::PushStyleColor(ImGuiCol_TableBorderLight, MenuConfig::table_border_color);
        ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, MenuConfig::table_border_color);
        ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, MenuConfig::table_header_bg_color);

        if (ImGui::BeginTable("##targets_table", 3,
                              ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollFreezeTopRow |
                              ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersVFullHeight,
                              {width - 2 * MenuConfig::window_padding.x,
                              ImGui::GetFrameHeight() * (rows + 0.4f)})) {
            /* Set up columns. */
            ImGui::TableSetupColumn("", 0, 0.175f); 
            ImGui::TableSetupColumn("Player name");
            ImGui::TableSetupColumn("Connection status");

            /* Set up column headers. */
            const float row_height = ImGui::GetTextLineHeight() +
                ImGui::GetStyle().CellPadding.y * 2.0f;
            ImGui::TableNextRow(ImGuiTableRowFlags_Headers, row_height);
            for (unsigned i = 0; 3 != i; ++i) {
                if (!ImGui::TableSetColumnIndex(i)) {
                    continue;
                }

                /* Add a checkmark, which we will use for multi-target
                * selection, in the first column instead of a label. */
                if (0 == i) {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0.0f, 0.0f});
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, MenuConfig::table_header_chkmrk_color);
                    bool b = targets.size() && targets.size() == selections.size();
                    if (ImGui::Checkbox("##", &b)) {
                        if (b) {
                            /* Add all target indices to ``selections``. */
                            selections.resize(targets.size());
                            std::iota(selections.begin(), selections.end(), 0);
                        } else {
                            selections.clear();
                        }
                    }
                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar();
                } else {
                    ImGui::TableHeader(ImGui::TableGetColumnName(i));
                }
            }

            /* Temporarily remove X-axis spacing in cells. */
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.0f,
                                ImGui::GetStyle().ItemSpacing.y});

            /* Populate columns. */
            for (decltype(selections)::value_type i = 0; targets.size() != i; ++i) {
                ImGui::TableNextRow();
                /* Check if the row indexed by ``i`` is selected. */
                const auto pos = std::find(selections.cbegin(), selections.cend(), i);
                bool row_selected = selections.cend() != pos;

                /* Slightly correct the Y value of the cursor position to
                * align the ``ImGui::Selectable`` properly. */
                ImGui::GetCurrentWindow()->DC.CursorPos.y += 1.0f;

                /* Add a selectable over the whole row so we're not limited
                * to checkbox-selection only. */
                ImGui::PushID(i);
                ImGui::PushStyleColor(ImGuiCol_Header, MenuConfig::table_row_color);
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
                                      MenuConfig::table_row_hover_color);
                ImGui::PushStyleColor(ImGuiCol_HeaderActive,
                                      MenuConfig::table_row_active_color);
                if (ImGui::Selectable("##", row_selected,
                                      ImGuiSelectableFlags_SpanAllColumns |
                                      ImGuiSelectableFlags_AllowItemOverlap)) {
                    if (row_selected) { selections.erase(pos); }
                    else { selections.push_back(i); }
                }
                ImGui::PopStyleColor(3);
                ImGui::PopID();
                ImGui::SameLine();

                ImGui::TableSetColumnIndex(0);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0.0f, 0.0f});
                ImGui::PushID(i);
                if (ImGui::Checkbox("##", &row_selected)) {
                    if (row_selected) { selections.push_back(i); }
                    else { selections.erase(pos); }
                }
                ImGui::PopID();
                ImGui::PopStyleVar();
                ImGui::TableSetColumnIndex(1);
                ImGui::Text(targets[i].name.c_str());
                ImGui::TableSetColumnIndex(2);
                /* If ``data`` is invalid, we assume the target is not
                * connected to the server. */
                ImGui::Text(targets[i].data ? "Online" : "Offline");
            }
            ImGui::PopStyleVar();
            ImGui::EndTable();
        }
        ImGui::PopStyleColor(3);

        ImGui::GetCurrentWindow()->DC.CursorPos.x = button_x;

        static bool in_selection = false;
        if (PlayerConstants::max_targets > targets.size() && !in_selection) {
            if (ImGui::Button("Add target")) {
                in_selection = true;
            }

            if (ImGui::IsItemHovered()) {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            }
        } else {
            ImGui::BeginDisabled();
            ImGui::Button("Add target");
            ImGui::EndDisabled();

            if (in_selection) {
                if (ImGui::IsItemHovered()) {
                    ImGui::ShowTooltip("Currently in target selection.",
                                       MenuConfig::tooltip_padding);
                }

                generate_player_info_window(&in_selection, &targets);
            } else if (ImGui::IsItemHovered()) {
                ImGui::ShowTooltip(("Maximum amount (" +
                                    std::to_string(PlayerConstants::max_targets) +
                                    ") of targets reached.").c_str(), MenuConfig::tooltip_padding);
            }
        }

        ImGui::SameLine();
        if (selections.size()) {
            if (ImGui::Button(("Remove selected (" +
                               std::to_string(selections.size()) + ')').c_str())) {
                for (auto index : selections) {
                    /* Reposition the selected indices which are in the interval
                     * [0; index). */
                    for (auto &id : selections) {
                        if (index < id) { --id; }
                    }

                    TargetManager::remove(targets.cbegin() + index);
                }

                selections.clear();
            }

            if (ImGui::IsItemHovered()) {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            }
        } else {
            ImGui::BeginDisabled();
            ImGui::Button("Remove selected");
            ImGui::EndDisabled();
        }
    ImGui::EndPanel();
}

void Menu::toggle(bool state)
{
    static POINT cursor_pos{0};

    if (state) {
        if (IS_SET(MenuConfig::settings, MenuConfig::remember_cursor_pos)) {
            ::GetCursorPos(&cursor_pos);
        }

        LocalPlayer::toggle_viewangle_mod(false);
        SDL_ShowCursor(1);
        ::SetCursor(::LoadCursorW(nullptr, IDC_ARROW));
    } else {
        if (IS_SET(MenuConfig::settings, MenuConfig::remember_cursor_pos) &&
            0 != cursor_pos.x && 0 != cursor_pos.y) {
            ::SetCursorPos(cursor_pos.x, cursor_pos.y);
        }

        LocalPlayer::toggle_viewangle_mod(true);
        SDL_ShowCursor(0);
    }
}

void Menu::init_addresses()
{
    SDL_ShowCursor =
        reinterpret_cast<func_SDL_ShowCursor>(Memory::get_export_address("SDL.dll",
                                                                         "SDL_ShowCursor"));
}

LRESULT CALLBACK Menu::h_WindowProc(HWND wnd,
                                    UINT msg,
                                    WPARAM wparam,
                                    LPARAM lparam)
{
    switch (msg) {
        case WM_LBUTTONDOWN: {
            AimbotConfig::can_aim = true;
            break;
        }

        case WM_LBUTTONUP: {
            AimbotConfig::can_aim = false;
            break;
        }

        case WM_KEYUP: { [[fallthrough]]; }
        case WM_SYSKEYUP: {
            switch (wparam) {
                case VK_ESCAPE: {
                    //::MessageBeep(MB_ICONERROR);
                    break;
                }
                
                case VK_INSERT: {
                    toggle((show = !show));
                    break;
                }
            }

            break;
        }
        
        default: { break; }
    }

    if (show && ImGui_ImplWin32_WndProcHandler(wnd, msg, wparam, lparam)) {
        return true;
    }

    return ::CallWindowProcA(old_wndproc, wnd, msg, wparam, lparam);
}