#include "rectangle.h"

Shapes::Rectangle &Shapes::Rectangle::scale(float s)
{
    const float scaled_width = top_left.distance(top_right) * s;
    const float scaled_height = bottom_left.distance(top_left) * s;

    top_left = {
        top_left[0] - scaled_width / 2,
        top_left[1] - scaled_height / 2
    };

    bottom_right = {
        bottom_right[0] + scaled_width / 2,
        bottom_right[1] + scaled_height / 2
    };

    top_right = {
        bottom_right[0],
        top_left[1]
    };

    bottom_left = {
        top_left[0],
        bottom_right[1]
    };

    return *this;
}