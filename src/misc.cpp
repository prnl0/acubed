#include "misc.h"

#include "config.h"
#include "memory.h"
#include "player.h"
#include "hookmanager.h"
#include "game.h"

bool Misc::fullbright = false;
bool Misc::rsk = false;
bool Misc::flying = false;
bool Misc::wallhack = false;
bool Misc::chams = false;

void Misc::toggle_flying(bool state,
                         MiscConfig::FlyingMode mode)
{
    /* We only want one mode active at any point in time. */
    toggle_flying_internal(false, MiscConfig::FlyingMode::ghost);
    toggle_flying_internal(false, MiscConfig::FlyingMode::inverse_minecraft);
    toggle_flying_internal(false, MiscConfig::FlyingMode::water);
    toggle_flying_internal(false, MiscConfig::FlyingMode::noclip);

    toggle_flying_internal(state, mode);
}

void Misc::toggle_wallhack(bool state)
{
    /* The following logic stems from analyzing the source code provided by
     * the developers at https://github.com/assaultcube/AC (specifically,
     * https://github.com/assaultcube/AC/blob/master/source/src/rendergl.cpp#L1012 and
     * https://github.com/assaultcube/AC/blob/master/source/src/rendermodel.cpp#L758).
     * It was not reverse-engineered, which it could have been (albeit with a
     * lot of trial and error). */

    static const auto gldisable_gl_fog = reinterpret_cast<void *>(Config::mod_base_address +
                                         Offsets::PatchRegions::gldisable_gl_fog);
    static const auto call_renderclients = reinterpret_cast<void *>(Config::mod_base_address +
                                           Offsets::PatchRegions::call_renderclients);

    if (state) {
        /* Replace a call to ``glDisable(GL_FOG)`` (which appears after the
         * calls to render other world objects) with a call to
         * ``render_clients`` in order to avoid problems with
         * ``glDisable(GL_DEPTH_TEST)`` (i. e. ``glEnable(GL_DEPTH_TEST)``
         * being ignored, and water, particles, etc. being drawn on top of
         * surfaces either way). */
        Memory::patch_bytes(gldisable_gl_fog, "\xE8\xCF\xE0\x00\x00\x90\x90", 7);

        /* Remove the previous call to ``render_clients``. */
        Memory::patch_bytes(call_renderclients, "\x90\x90\x90\x90\x90", 5);

        HookManager::hook("render_clients",
                          reinterpret_cast<void *>(Offsets::Function::render_clients),
                          Game::h_render_clients, &Game::render_clients_gate, 6).enable();


    } else {
        /* Restore original states. */
        Memory::patch_bytes(gldisable_gl_fog, "\x68\x60\x0B\x00\x00\xFF\xD7", 7);
        Memory::patch_bytes(call_renderclients, "\xE8\xB3\xE1\x00\x00", 5);
        HookManager::disable("render_clients");
    }
}

void Misc::toggle_flying_internal(bool state,
                                  MiscConfig::FlyingMode mode)
{
    static const auto disable_flying_addr = reinterpret_cast<void *>(Config::mod_base_address +
                                            Offsets::PatchRegions::disable_flying);
    static const auto water_level = reinterpret_cast<std::uint32_t *>(Config::mod_base_address +
                                    Offsets::Module::water_level);

    if (state) {
        switch (mode) {
            case MiscConfig::FlyingMode::inverse_minecraft: {
                Memory::patch_bytes(disable_flying_addr, "\xC6\x40\x6A\x01", 4);
                break;
            }

            case MiscConfig::FlyingMode::water: {
                *water_level = 50;
                break;
            }
            
            default: {
                break;
            }
        }
    } else {
        switch (mode) {
            case MiscConfig::FlyingMode::ghost: {
                LocalPlayerConfig::ptr->set_state2(0);
                break;
            }

            case MiscConfig::FlyingMode::inverse_minecraft: {
                Memory::patch_bytes(disable_flying_addr, "\xC6\x40\x6A\x00", 4);
                break;
            }

            case MiscConfig::FlyingMode::water: {
                *water_level = 0xFFFFFFF6;
                break;
            }

            case MiscConfig::FlyingMode::noclip: {
                LocalPlayerConfig::ptr->set_state1(0);
                break;
            }
        }
    }
}