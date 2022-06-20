#include "angle.h"

#include <cmath>

#include "vector.h"

Vector<float, 3> Angle::compute_angle(const Vector<float, 3> &src,
                                      const Vector<float, 3> &dst)
{
    return {-std::atan2f(dst[0] - src[0], dst[1] - src[1]) / pi * 180.0f + 180.0f,  // Pitch
            std::asinf((dst[2] - src[2]) / src.distance(dst)) / pi * 180.0f,        // Yaw
            0.0f};                                                                  // Roll
}

float Angle::normalize_angle(float angle)
{
    /* Rounds down to one digit after the decimal point. */
    return std::round(
        std::abs(360.0f + angle - static_cast<int>(360.0f + angle) / 360 * 360) * 10) / 10;
}

bool Angle::is_in_range(float center_angle,
                        float target_angle,
                        float range)
{
    const float min_angle = normalize_angle(center_angle - range / 2);
    const float max_angle = normalize_angle(center_angle + range / 2);

    return min_angle >= max_angle ?
        (min_angle <= target_angle || max_angle >= target_angle) :
        (min_angle <= target_angle && max_angle >= target_angle);
}