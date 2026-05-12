#pragma once
#include "raylib.h"
#include <cmath>

template<typename T = float>
struct Vec3 {
    T x, y, z;

    constexpr Vec3() : x(0), y(0), z(0) {}
    constexpr Vec3(T x, T y, T z) : x(x), y(y), z(z) {}
    constexpr Vec3(Vector3 v) : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)), z(static_cast<T>(v.z)) {}
    template<typename U>
    constexpr Vec3(const Vec3<U>& o) : x(static_cast<T>(o.x)), y(static_cast<T>(o.y)), z(static_cast<T>(o.z)) {}

    Vector3 v() const { return {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)}; }
    operator Vector3() const { return {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)}; }

    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator-()            const { return {-x, -y, -z}; }
    Vec3 operator*(T s)         const { return {x * s, y * s, z * s}; }
    Vec3 operator/(T s)         const { return {x / s, y / s, z / s}; }

    Vec3& operator+=(const Vec3& o) {
        x += o.x;
        y += o.y;
        z += o.z;
        return *this;
    }
    Vec3& operator-=(const Vec3& o) {
        x -= o.x;
        y -= o.y;
        z -= o.z;
        return *this;
    }
    Vec3& operator*=(T s) {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }
    Vec3& operator/=(T s) {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }

    template<typename S>
    Vec3<S> as() const { return {static_cast<S>(x), static_cast<S>(y), static_cast<S>(z)}; }

    T    dot(const Vec3& o)   const { return x * o.x + y * o.y + z * o.z; }
    Vec3 cross(const Vec3& o) const { return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x}; }
    T    len()                const { return std::sqrt(x * x + y * y + z * z); }
    T    lenSqr()             const { return x * x + y * y + z * z; }
    Vec3 norm()               const { T l = len(); return l > 0 ? *this / l : Vec3{}; }

    T  operator[](int i) const { return (&x)[i]; }
    T& operator[](int i)       { return (&x)[i]; }

    static const Vec3 Origin;
    static const Vec3 Up;
};

template<typename T>
const Vec3<T> Vec3<T>::Origin{0, 0, 0};
template<typename T>
const Vec3<T> Vec3<T>::Up{0, 1, 0};

template<typename T>
inline Vec3<T> operator*(T s, const Vec3<T>& v) { return v * s; }

using Vec3f = Vec3<float>;
using Vec3i = Vec3<int>;
