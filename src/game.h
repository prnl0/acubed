#pragma once

#ifndef GAME_GUARD
#define GAME_GUARD

#include "vector.h"
#include "constants.h"
#include "player.h"
#include "matrix.h"

namespace Game
{
    struct TraceResult
    {
        Vector<float, 3> end{0.0f, 0.0f, 0.0f};
        bool has_collided = false;
    };

    /* In-game function addresses */
    constexpr auto trace_line = Offsets::Function::traceline;

    /* Gateways (intermediaries used to execute stolen bytes when hooking) */
    extern void (*render_clients_gate)();
    extern char (*on_gamestart_gate)(const char *, const char *);

    /* Hooked functions */
    void h_render_clients();
    char h_on_gamestart(const char *a1,
                        const char *a2);

    /* Re-implemented in-game functions */
    TraceResult trace(const Vector<float, 3> &src,
                      const Vector<float, 3> &dst,
                      const void *trace_owner);

    Vector<float, 2> world_to_screen(const Matrix<float, 4, 4> &mvp_matrix,
                                     const Vector<float, 4> &ent_coords);

    void fullbright(bool state,
                    const Vector<float, 3> &color = {1.0f, 1.0f, 1.0f},
                    bool overwrite_defaults = true);
} // namespace Game

#endif // ifndef GAME_GUARD