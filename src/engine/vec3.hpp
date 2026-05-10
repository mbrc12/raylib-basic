#pragma once
#include "raylib.h"
#include <cmath>

struct Vec3 {
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3(Vector3 v) : x(v.x), y(v.y), z(v.z) {}

    Vector3 v() const { return {x, y, z}; }

    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator-()            const { return {-x, -y, -z}; }
    Vec3 operator*(float s)     const { return {x * s, y * s, z * s}; }
    Vec3 operator/(float s)     const { return {x / s, y / s, z / s}; }

    float  dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    float  len()              const { return sqrtf(x * x + y * y + z * z); }
    Vec3   norm()             const { float l = len(); return l > 0 ? *this / l : Vec3{}; }

    float  operator[](int i) const { return (&x)[i]; }
    float& operator[](int i)       { return (&x)[i]; }
};
inline Vec3 operator*(float s, const Vec3& v) { return v * s; }
