#pragma once

#ifndef VECTOR_GUARD
#define VECTOR_GUARD

#include <cstdint>
#include <array>
#include <cmath>

/* Used in (currently) one peculiar scenario when calling traceline through
 * inline assembly. */
struct Vector3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

/* FIXME: fix this mess. */
template<class T, std::uint8_t N>
struct Vector
{
    using size_type = std::uint8_t;
    using iterator = T *;
    using const_iterator = const T *;

    std::array<T, N> data;

    float distance(const Vector<T, N> &dst) const { return (*this - dst).magnitude(); }
    float magnitude() const
    {
        float magn = 0.0f;
        for (auto k : data) { magn += std::powf(k, 2); }
        return std::sqrtf(magn);
    }

    auto begin() noexcept { return data.begin(); }
    auto end() noexcept { return data.end(); }

    const_iterator begin() const noexcept { return data.begin(); }
    const_iterator end() const noexcept { return data.end(); }

    const_iterator cbegin() const noexcept { return data.cbegin(); }
    const_iterator cend() const noexcept { return data.cend(); }

    const T &operator[](size_type i) const { return data[i]; }
    T &operator[](size_type i) { return data[i]; }

    template<typename T_op>
    Vector<T, N> operator+(T_op n) const
    {
        Vector<T, N> vec = *this;
        for (size_type i = 0; N != i; ++i) { vec[i] += n; }
        return vec;
    }

    Vector<T, N> operator+(const Vector<T, N> &rhs) const
    {
        Vector<T, N> vec = *this;
        for (size_type i = 0; N != i; ++i) { vec[i] += rhs[i]; }
        return vec;
    }

    Vector<T, N> operator-(T n) const
    {
        Vector<T, N> vec = *this;
        for (auto k : vec) { k -= n; }
        return vec;
    }

    Vector<T, N> operator-(const Vector<T, N> &rhs) const
    {
        Vector<T, N> vec = *this;
        for (size_type i = 0; N != i; ++i) { vec[i] -= rhs[i]; }
        return vec;
    }
        
    template<typename T_op>
    Vector<T, N> operator/(T_op n) const
    {
        Vector<T, N> vec = *this;
        for (size_type i = 0; N != i; ++i) { vec[i] /= n; }
        return vec;
    }

    template<typename T_op>
    Vector<T, N> &operator/=(T_op n)
    {
        for (auto &d : data) {
            d /= n;
        }
        return *this;
    }
};

#endif // VECTOR_GUARD