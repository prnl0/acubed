#pragma once

#ifndef MATRIX_GUARD
#define MATRIX_GUARD

#include <cstddef>
#include <array>

#include "vector.h"

/* TODO: fix this. */

template<class T, std::uint8_t R, std::uint8_t C>
struct Matrix
{
    std::array<float, C * R> data;

    Vector<T, R> operator*(const Vector<T, C> &rhs) const
    {
        Vector<T, R> res;
        for (std::uint8_t i = 0; R != i; ++i) {
            res[i] = 0;
            for (std::uint8_t j = 0; C != j; ++j) {
                res[i] += data[j * C + i] * rhs[j];
            }
        }
        return res;
    }
};

#endif // ifndef MATRIX_GUARD