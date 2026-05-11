#pragma once
#include "raylib.h"
#include <cmath>

template<typename T = float>
struct Vec2 {
    T x, y;

    constexpr Vec2() : x(0), y(0) {}
    constexpr Vec2(T x, T y) : x(x), y(y) {}
    constexpr Vec2(Vector2 v) : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)) {}
    template<typename U>
    constexpr Vec2(const Vec2<U>& o) : x(static_cast<T>(o.x)), y(static_cast<T>(o.y)) {}

    Vector2 v() const { return {static_cast<float>(x), static_cast<float>(y)}; }
    operator Vector2() const { return {static_cast<float>(x), static_cast<float>(y)}; }

    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator-()            const { return {-x, -y}; }
    Vec2 operator*(T s)         const { return {x * s, y * s}; }
    Vec2 operator/(T s)         const { return {x / s, y / s}; }

    template<typename S>
    Vec2<S> as() const { return {static_cast<S>(x), static_cast<S>(y)}; }

    T    dot(const Vec2& o) const { return x * o.x + y * o.y; }
    T    len()              const { return std::sqrt(x * x + y * y); }
    T    lenSqr()           const { return x * x + y * y; }
    Vec2 norm()             const { T l = len(); return l > 0 ? *this / l : Vec2{}; }

    T  operator[](int i) const { return (&x)[i]; }
    T& operator[](int i)       { return (&x)[i]; }

    static const Vec2 Origin;
    static const Vec2 Right;
    static const Vec2 Up;
};

template<typename T>
const Vec2<T> Vec2<T>::Origin{0, 0};
template<typename T>
const Vec2<T> Vec2<T>::Right{1, 0};
template<typename T>
const Vec2<T> Vec2<T>::Up{0, 1};

template<typename T>
inline Vec2<T> operator*(T s, const Vec2<T>& v) { return v * s; }

using Vec2f = Vec2<float>;
using Vec2i = Vec2<int>;
