#pragma once

#ifndef MENU_GUARD
#define MENU_GUARD

#include <vector>
#include <string>

#include <Windows.h>

#include "config.h"
#include "imgui/imgui.h"
#include "imgui/imgui_extensions.h"
#include "target.h"

class Player;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

class Menu
{
    public:
        Menu() = delete;

        static bool enable();
        static bool disable();

        static void generate_drawlist();

        static bool is_shown() { return show; }

    private:
        static void generate_info_window();
        static void generate_main_window();
        static void generate_player_info_window(bool *in_selection = nullptr,
                                                std::vector<Target> *targets = nullptr);

        static void generate_categories_sub();
        static void generate_aimbot_sub();
        static void generate_esp_sub();
        static void generate_triggerbot_sub();
        static void generate_misc_sub();
        static void generate_styling_sub();
        static void generate_load_save_sub();

        static void generate_target_sel_panel(const float width,
                                              const unsigned rows = 5);

        static void toggle(bool state);
        static void init_addresses();
        
        static LRESULT CALLBACK h_WindowProc(HWND wnd,
                                             UINT msg,
                                             WPARAM wparam,
                                             LPARAM lparam);

        /* ImGui wrappers. */
        static void help_marker(const std::string &desc)
        {
            ImGui::HelpMarker(desc.c_str(), MenuConfig::tooltip_padding);
        }

        static bool enabled;
        static bool show;

        static decltype(MenuConfig::categories)::size_type selected_index;
        static ImVec2 scale;
        static ImVec2 size;

        static WNDPROC old_wndproc;

        static int (*SDL_ShowCursor)(int);
};

#endif // ifndef MENU_GUARD