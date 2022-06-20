#pragma once

#ifndef ANGLE_GUARD
#define ANGLE_GUARD

#include "vector.h"

namespace Angle
{
    constexpr float pi = 3.14159f;

    Vector<float, 3> compute_angle(const Vector<float, 3> &src,
                                   const Vector<float, 3> &dst);

    float normalize_angle(float angle);

    bool is_in_range(float center_angle,
                     float target_angle,
                     float range);
} // namespace Angle

#endif // ifndef ANGLE_GUARD