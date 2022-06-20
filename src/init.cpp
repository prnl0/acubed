#include "init.h"

#include <system_error>

#include <Windows.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_opengl2.h"
#include "imgui/imgui_extensions.h"

#include "player.h"
#include "entity.h"
#include "aimbot.h"
#include "esp.h"
#include "misc.h"
#include "menu.h"
#include "config.h"
#include "hookmanager.h"
#include "constants.h"
#include "game.h"
#include "weapon.h"

BOOL (__stdcall *wsb_gate)(HDC) = nullptr;

BOOL __stdcall h_wglSwapBuffers(HDC dc)
{
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
        ImGui::BeginCanvas();

        /* TRY-FIXME: ~20-30 FPS lost to this (at least it seems like it in
         * some circumstances). */
        
        /* Resort to constant player list repopulation because by hooking
         * "``display_to_chat``" function I was only able to handle
         * single-to-multi, multi-to-single, and multi-to-multi
         * connections/disconnections but not single-to-single, which would
         * throw a memory read access violation when trying to update weapon
         * data. This could be achieved by employing padded structures which
         * rely on reading a set amount of bytes, rather than extracting data
         * from individual addresses, but I'd rather not occupy more memory
         * than is needed. Nonetheless, the performance impact is negligible,
         * therefore there is no point in giving this more thought. */
        PlayerList::repopulate();
        EntityList::repopulate();
        Menu::generate_drawlist();

        if (!LocalPlayerConfig::ptr->is_dead()) {
            if (Aimbot::is_enabled() && (IS_SET(AimbotConfig::settings, AimbotConfig::auto_aim) ||
                                         AimbotConfig::can_aim)) {
                Aimbot::aim();
            }

            if (ESP::is_player_esp_enabled()) {
                ESP::draw_player_esp();
            }

            if (ESP::is_entity_esp_enabled()) {
                ESP::draw_entity_esp();
            }

            if (Misc::is_flying()) {
                if (MiscConfig::FlyingMode::ghost == MiscConfig::flying_mode) {
                    LocalPlayerConfig::ptr->set_state2(5);
                } else if (MiscConfig::FlyingMode::noclip == MiscConfig::flying_mode) {
                    LocalPlayerConfig::ptr->set_state1(4);
                }
            }
        }
        ImGui::EndCanvas();
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    return wsb_gate(dc);
}

void Init::init(const HWND &hwnd)
{
    ImGui::Init(hwnd);

    auto &mod_addr = Config::mod_base_address;
    mod_addr = reinterpret_cast<Config::addr>(::GetModuleHandleA("ac_client.exe"));
    Config::ticks = reinterpret_cast<std::uint32_t *>(mod_addr + Offsets::Module::ticks);
    Config::hwnd = hwnd;

    while (!(LocalPlayerConfig::ptr = *reinterpret_cast<LocalPlayer **>(Config::mod_base_address +
                                      Offsets::Module::local_player_pointer))) {}

    using lpc = LocalPlayerConfig;
    lpc::viewmatrix_address         = mod_addr + Offsets::Module::viewmatrix;
    lpc::fov_address                = mod_addr + Offsets::Module::fov;
    lpc::show_blocked_icon_address  = mod_addr + Offsets::Module::show_blocked_icon;
    lpc::call_viewangle_mod_address = mod_addr + Offsets::PatchRegions::call_viewangle_mod;
    lpc::weapon_list_address        = reinterpret_cast<Config::addr>(LocalPlayerConfig::ptr) +
                                      Offsets::Player::weapon_list;

    /* TODO: eventually move to a function named ``load_settings`` (or so). */
    if (Aimbot::is_enabled() && IS_SET(AimbotConfig::settings, AimbotConfig::auto_attack)) {
        Weapon::toggle_auto_attack_all(true);
    }

    if (!Menu::enable()) {
        throw std::system_error(::GetLastError(), std::system_category(),
                                "Init::init: failed to enable the menu.");
    }

    HookManager::hook("on_gamestart", reinterpret_cast<void *>(Offsets::Function::on_gamestart),
                      Game::h_on_gamestart, &Game::on_gamestart_gate, 12).enable();
    HookManager::hook("opengl32.dll", "wglSwapBuffers", h_wglSwapBuffers, &wsb_gate, 5).enable();
}

void Init::shutdown()
{
    /* Reset in-game variables, patched regions. */
    Game::fullbright(false);
    Weapon::toggle_consec_shots(true);
    Weapon::reset_rsk();
    Misc::toggle_flying(false, MiscConfig::FlyingMode::ghost);

    Menu::disable();
    HookManager::disable_all();
    ::Sleep(250);
    ImGui::Shutdown();
}