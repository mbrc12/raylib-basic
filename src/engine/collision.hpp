#pragma once
#include "vec3.hpp"

struct Box {
    Vec3  center;
    float halfX, halfY, halfZ;
};

struct SweepResult {
    float t;          // collision time [0,1]; negative if already intersecting
    Vec3  pos;        // box center at collision (face-snapped)
    Vec3  normal;     // points from other box toward moving box
    bool  intersect;  // true if boxes were already overlapping at start
};

SweepResult sweep(const Box& body, Vec3 target, const Box& other);
