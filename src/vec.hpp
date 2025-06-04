#pragma once

#include <cstddef>
#include <type_traits>

template <typename T, size_t N>
struct Vector {
    using Self = Vector<T, N>;

    T elements[N];

    Vector() = default;

    template <typename... Args>
    requires(sizeof...(Args) == N && std::conjunction_v<std::is_convertible<Args, T>...>)
    Vector(Args... args) : elements{static_cast<T>(args)...} {}

    template <typename U>
        requires std::is_convertible_v<U, T>
    Vector(const Vector<U, N>& other) {
        for (size_t i = 0; i < N; ++i) {
            elements[i] = static_cast<T>(other.elements[i]);
        }
    }

    template <typename U>
        requires std::is_convertible_v<U, T>
    Self operator+(const Vector<U, N>& other) {
        Self res;
        for (int i = 0; i < N; i++) {
            res.elements[i] = elements[i] + static_cast<T>(other.elements[i]);
        }
    }

    template <typename U>
        requires std::is_convertible_v<U, T>
    Self operator-(const Vector<U, N>& other) {
        Self res;
        for (int i = 0; i < N; i++) {
            res.elements[i] = elements[i] - static_cast<T>(other.elements[i]);
        }
    }

    template <typename U>
        requires std::is_convertible_v<U, T>
    Self operator*(const Vector<U, N>& other) {
        Self res;
        for (int i = 0; i < N; i++) {
            res.elements[i] = elements[i] * static_cast<T>(other.elements[i]);
        }
    }

    template <typename U>
        requires std::is_convertible_v<U, T>
    Self operator/(const Vector<U, N>& other) {
        Self res;
        for (int i = 0; i < N; i++) {
            res.elements[i] = elements[i] / static_cast<T>(other.elements[i]);
        }
    }
};

using vec2 = Vector<float, 2>;
using vec3 = Vector<float, 3>;
using vec4 = Vector<float, 4>;

using ivec2 = Vector<int, 2>;
using ivec3 = Vector<int, 3>;
using ivec4 = Vector<int, 4>;

using uvec2 = Vector<unsigned int, 2>;
using uvec3 = Vector<unsigned int, 3>;
using uvec4 = Vector<unsigned int, 4>;
