#pragma once

#ifndef RECTANGLE_GUARD
#define RECTANGLE_GUARD

#include "vector.h"

namespace Shapes
{
    struct Rectangle
    {
        Vector<float, 2> top_left{0.0f, 0.0f};
        Vector<float, 2> top_right{0.0f, 0.0f};
        Vector<float, 2> bottom_right{0.0f, 0.0f};
        Vector<float, 2> bottom_left{0.0f, 0.0f};

        Rectangle &scale(float s);
    };
} // namespace Shapes

#endif // RECTANGLE_GUARD