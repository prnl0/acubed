#include "aimbot.h"

#include <memory>

#include "player.h"
#include "config.h"
#include "menu.h"
#include "target.h"
#include "angle.h"

bool Aimbot::enabled = false;
bool Aimbot::prev_attacking_state = false;
Player *Aimbot::target = nullptr;

void Aimbot::aim()
{
    const auto settings = AimbotConfig::settings;
    auto &lp = *LocalPlayerConfig::ptr;

    /* Aim only if:
     *     - ``disable_on_reload`` is disabled or the player isn't reloading, and
     *     - the player has at least one bullet in either their clip or magazine. */
    if ((!IS_SET(settings, AimbotConfig::disable_on_reload) || !lp.is_reloading()) &&
        (0 != lp.get_current_weapon()->get_clip() || 0 != lp.get_current_weapon()->get_mag())) {
        /* Look for a new target if ``lock_on_target`` isn't enabled or the
         * current one isn't valid anymore. */
        if (!IS_SET(settings, AimbotConfig::lock_on_target) || !is_player_valid(target) ) {
            target = find_target();
        }

        if (target) {
            if (IS_SET(settings, AimbotConfig::auto_aim)) {
                /* Disable mouse if there is a target in the FOE (Field Of Effect). */
                if (IS_SET(settings, AimbotConfig::disable_mouse)) {
                    LocalPlayer::toggle_viewangle_mod(false);
                }

                /* TODO: maybe enable triggerbot when smoothing is enabled as
                 *       otherwise the player will start shooting before
                 *       actually aiming at the target. */
                if (IS_SET(settings, AimbotConfig::auto_attack) &&
                    "grenade" != lp.get_current_weapon()->get_name()) {
                    lp.set_is_attacking(prev_attacking_state = true);
                }

                if (IS_SET(settings, AimbotConfig::smoothing_enabled)) {
                    /* Compute the amount of rotation needed to reach target
                     * angle. */
                    auto diff = Angle::compute_angle(lp.get_position(), target->get_position()) -
                                lp.get_viewangles();

                    /* Divide it into uniform steps as defined per the
                     * ``smoothing`` variable. */
                    diff /= AimbotConfig::smoothing;
                    lp.set_viewangles(lp.get_viewangles() + diff);
                }
            }

            /* Move this out here to avoid doing two additional checks for
             * ``auto_aim``. */
            if (!IS_SET(settings, AimbotConfig::smoothing_enabled)) {
                lp.set_viewangles(Angle::compute_angle(lp.get_position(),
                                                       target->get_position()));
            }

            /* NOTE: all four ``toggle_blocked_icon_visibility`` calls are
             *       needed as they handle four different scenarios:
             *           1. The hack loads with ``hide_blocked`` enabled, thus
             *              requiring us to hide the icon on initialization.
             *           2. The aimbot gets toggled on/off, thus requiring us
             *              to switch between hiding and showing the icon.
             *           3. ``aim_at_teammates`` gets toggled on/off, thus
             *              requiring us to switch between hiding and showing
             *              the icon.
             *           4. ``hide_blocked`` gets toggled on/off, thus
             *              requiring us to switch between hiding and showing
             *              the icon.
             *       Although it may be possible to further improve upon this.
             */
            if (IS_SET(AimbotConfig::settings, AimbotConfig::aim_at_teammates) &&
                IS_SET(AimbotConfig::settings, AimbotConfig::hide_blocked)) {
                LocalPlayer::toggle_blocked_icon_visibility(false);
            } else {
                LocalPlayer::toggle_blocked_icon_visibility(true);
            }
        } else if (!Menu::is_shown()) {
            /* Enable mouse to restore intended behaviour. */
            LocalPlayer::toggle_viewangle_mod(true);
            LocalPlayer::toggle_blocked_icon_visibility(true);
            
            /* Check if we previously set the players' state to "attacking" as
             * to not interfere with genuine shooting, slashing, etc. */
            if (prev_attacking_state) {
                lp.set_is_attacking(prev_attacking_state = false);
            }
        }
    } else if (!Menu::is_shown()) {
        /* Enable mouse to handle various cases when it stays disabled. */
        LocalPlayer::toggle_viewangle_mod(true);
        LocalPlayer::toggle_blocked_icon_visibility(true);
    }
}

Player *Aimbot::find_target()
{
    Player *target = nullptr;
    float distance_to_target = 9999.0f;

    /* First, loop through the target list to see if there any valid
     * candidates. */
    for (const auto &t : TargetManager::get_all()) {
        if (is_player_valid(t.data, &distance_to_target)) {
            target = t.data;
        }
    }

    /* If not, and we only prioritize targets (not isolate them), try to look
     * for a valid candidate in the whole player list. */
    if (IS_SET(AimbotConfig::target_settings, AimbotConfig::prioritize_targets) && !target) {
        for (const auto &p : PlayerList::get_list()) {
            if (is_player_valid(p, &distance_to_target)) {
                target = p;
            }
        }
    }

    return target;
}

bool Aimbot::is_player_valid(const Player *p,
                             float *distance_to_target)
{
    /* If ``p`` is null, the player isn't online and is therefore invalid. */
    if (!p) {
        return false;
    }

    const auto settings = AimbotConfig::settings;
    const auto &lp = *LocalPlayerConfig::ptr;

    /* Player is not valid if:
     *     - they are dead;
     *     - we don't want to aim at teammates and they are in the same
     *       team;
     *     - they are in the spectators. */
    if (p->is_dead() ||
        (!IS_SET(settings, AimbotConfig::aim_at_teammates) && lp.get_team() == p->get_team())
        || (p->get_team() == PlayerConstants::Team::spectator)) {
        return false;
    }

    /* Player is not valid if the target is not in the FOE. */
    if (!Angle::is_in_range(lp.get_viewangles()[0],
                            Angle::compute_angle(lp.get_position(), p->get_position())[0],
                            AimbotConfig::foe)) {
        return false;
    }

    /* Player is not valid if we don't want to shoot through walls and we
     * can't see them. */
    if (!IS_SET(settings, AimbotConfig::aim_through_walls) && !lp.can_see(*p)) {
        return false;
    }

    if (distance_to_target) {
        /* Player is not valid if they are farther away than the previous
         * player. */
        const float distance_to_p = lp.get_position().distance(p->get_position());
        if (distance_to_p > *distance_to_target) {
            return false;
        }

        *distance_to_target = distance_to_p;
    }

    return true;
}