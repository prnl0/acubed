#pragma once

#ifndef ESP_GUARD
#define ESP_GUARD

#include <string> // string

#include "imgui/imgui.h"

#include "vector.h"
#include "rectangle.h"

class Player;

class ESP
{
    friend class Menu;

    public:
        ESP() = delete;

        static void draw_player_esp();
        static void draw_entity_esp();

        static void toggle_player_esp(bool state) { player_esp_enabled = state; }

        static bool is_player_esp_enabled() { return player_esp_enabled; }
        static bool is_entity_esp_enabled() { return entity_esp_enabled; }

    private:
        static bool is_player_valid(const Player *p);

        static void draw_player(const Player *player,
                                bool show_additional_info);
        static Shapes::Rectangle draw_hp_ap_bars(const Player *player);
        static void draw_reload_bar(const Player *player);
        static void draw_player_add_info(const Player *player,
                                         ImU32 border_color);

        static void draw_esp_box(const Vector<float, 2> center,
                                 float scale,
                                 ImU32 color,
                                 bool player_box);
        static void draw_entity_label(const Vector<float, 2> &center,
                                      const std::string &label,
                                      ImU32 color);
        
        static bool player_esp_enabled;
        static bool entity_esp_enabled;

        /* Bounding box of the current ESP box. Used to position elements
         * around it. */
        static Shapes::Rectangle esp_box;
};

#endif // ifndef ESP_GUARD