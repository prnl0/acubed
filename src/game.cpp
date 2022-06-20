#include "game.h"

#pragma comment(lib, "OpenGL32.lib")

#include <cstdarg>
#include <vector>
#include <string>

#include <Windows.h>
#include <gl/GL.h>

#include "aimbot.h"
#include "config.h"
#include "vector.h"
#include "player.h"
#include "matrix.h"
#include "constants.h"
#include "misc.h"

void (*Game::render_clients_gate)() = nullptr;
char (*Game::on_gamestart_gate)(const char *, const char *) = nullptr;

void Game::h_render_clients()
{
    /* The following logic stems from analyzing the source code provided by
     * the developers at https://github.com/assaultcube/AC (specifically,
     * https://github.com/assaultcube/AC/blob/master/source/src/rendergl.cpp#L1012 and
     * https://github.com/assaultcube/AC/blob/master/source/src/rendermodel.cpp#L758).
     * It was not reverse-engineered, which it could have been (albeit with a
     * lot of trial and error). */

    static const auto clients_rad = reinterpret_cast<void *>(Config::mod_base_address +
                                                             Offsets::Module::clients_rad);

    /* Acquire permissions to write a negative value to ``rad``, which allows
     * us to bypass occlusion checks. */
    DWORD old_protect = 0;
    if (!::VirtualProtect(clients_rad, 4, PAGE_EXECUTE_READWRITE, &old_protect)) {
        /* Early out if we failed to acquire the necessary permissions as the
         * game will crash otherwise. */
        return render_clients_gate();
    }

    *reinterpret_cast<float *>(clients_rad) = -1.0f;

    /* Disable depth-testing to allow overdrawing. */
    glDisable(GL_DEPTH_TEST);

    render_clients_gate();

    /* Restore initial state. */
    glEnable(GL_DEPTH_TEST);
    *reinterpret_cast<float *>(clients_rad) = 1.5f;
    ::VirtualProtect(clients_rad, 4, old_protect, &old_protect);
}

char Game::h_on_gamestart(const char *a1,
                          const char *a2)
{
    /* Reset fullbright if it's enabled since the lights get updated on map
     * change. */
    if (Misc::is_fullbright()) {
        fullbright(true, MiscConfig::fullbright_col);
    }

    return on_gamestart_gate(a1, a2);
}

/* Source: https://guidedhacking.com/threads/how-to-find-traceline-call-traceline-with-inline-asm.6695/ */
Game::TraceResult Game::trace(const Vector<float, 3> &src,
                              const Vector<float, 3> &dst,
                              const void *trace_owner)
{
    const Vector3 d{dst[0], dst[1], dst[2]};
    const Vector3 s{src[0], src[1], src[2]};

    TraceResult trace_result;
    __asm {
        push 0
        push 0
        push trace_owner
        push d.z
        push d.y
        push d.x
        push s.z
        push s.y
        push s.x
        lea eax, [trace_result]
        call trace_line
        add esp, 36
    }
    return trace_result;
}

/* Basic explanation:
 *     - https://learnopengl.com/Getting-started/Transformations
 *     - https://learnopengl.com/Getting-started/Coordinate-Systems */
Vector<float, 2> Game::world_to_screen(const Matrix<float, 4, 4> &mvp_matrix,
                                       const Vector<float, 4> &ent_coords)
{
    const auto clip_coords = mvp_matrix * ent_coords;

    const Vector<float, 3> ndc{
        clip_coords[0] / clip_coords[3],
        clip_coords[1] / clip_coords[3],
        clip_coords[2] / clip_coords[3]
    };

    const auto &io = ImGui::GetIO();
    return {
         io.DisplaySize.x / 2 * ndc[0] + ndc[0] + io.DisplaySize.x / 2,
        -io.DisplaySize.y / 2 * ndc[1] + ndc[1] + io.DisplaySize.y / 2
    };
}

void Game::fullbright(bool state,
                      const Vector<float, 3> &color,
                      bool overwrite_defaults)
{
    const static auto fb_list_addr = reinterpret_cast<std::uintptr_t *>(
                                     Config::mod_base_address + Offsets::Module::fullbright_list);
    const static auto fb_num_addr = Config::mod_base_address + Offsets::Module::fullbright_num;

    std::uint8_t *col = reinterpret_cast<std::uint8_t *>(*fb_list_addr + 0x6);
    const auto count = *reinterpret_cast<std::uint32_t *>(fb_num_addr);

    static std::vector<Vector<std::uint8_t, 3>> defaults;

    if (state) {
        /* We need to retrieve the initial values of the lights so we are able
         * to restore them later if needed. We do so on each
         * ``fullbright(true, ..., true)`` invocation as to prevent failure
         * upon map change. */
        if (overwrite_defaults) {
            defaults.clear();
            for (std::uint32_t i = 0; count != i; ++i, col += 0x10) {
                defaults.push_back({col[0], col[1], col[2]});
            }
        }

        for (std::uint32_t i = 0; count != i; ++i, col += 0x10) {
            col[0] = static_cast<std::uint8_t>(color[0] * 255); // Red
            col[1] = static_cast<std::uint8_t>(color[1] * 255); // Green
            col[2] = static_cast<std::uint8_t>(color[2] * 255); // Blue
        }
    } else if (0 != defaults.size()) {
        for (std::uint32_t i = 0; count != i; ++i, col += 0x10) {
            col[0] = defaults[i][0];
            col[1] = defaults[i][1];
            col[2] = defaults[i][2];
        }
    }
}