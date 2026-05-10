#pragma once
#include "raylib.h"
#include <cmath>

struct Box {
    Vector3 center;
    float halfX, halfY, halfZ;
};

struct SweepResult {
    float t;          // fraction [0,1] where collision occurs, 1 if none
    Vector3 pos;      // box center at collision
    Vector3 normal;   // points from other box toward moving box
    bool intersect;   // true if boxes already overlapping at start
};

namespace {

void axis(float bc, float bh, float oc, float oh, float v,
          float& t_entry, float& t_exit) {
    float d_entry = oc - oh - bc - bh; // body max reaches other min
    float d_exit  = oc + oh - bc + bh; // body min reaches other max

    if (fabsf(v) < 1e-6f) {
        if (d_entry > 0.0f || d_exit < 0.0f) {
            t_entry = 1.0f;
            t_exit  = -1.0f;
        } else {
            t_entry = -INFINITY;
            t_exit  = INFINITY;
        }
        return;
    }

    t_entry = d_entry / v;
    t_exit  = d_exit / v;
    if (v < 0.0f) {
        float tmp = t_entry;
        t_entry = t_exit;
        t_exit  = tmp;
    }
}

} // namespace

inline SweepResult sweep(const Box& body, Vector3 target, const Box& other) {
    Vector3 v = Vector3Subtract(target, body.center);

    float tx_e, tx_x, ty_e, ty_x, tz_e, tz_x;
    axis(body.center.x, body.halfX, other.center.x, other.halfX, v.x, tx_e, tx_x);
    axis(body.center.y, body.halfY, other.center.y, other.halfY, v.y, ty_e, ty_x);
    axis(body.center.z, body.halfZ, other.center.z, other.halfZ, v.z, tz_e, tz_x);

    float t_enter = fmaxf(fmaxf(tx_e, ty_e), tz_e);
    float t_exit  = fminf(fminf(tx_x, ty_x), tz_x);

    if (t_enter > t_exit || t_exit < 0.0f || t_enter > 1.0f)
        return {1.0f, target, {0, 0, 0}, false};

    float t = fmaxf(0.0f, t_enter);
    bool  intersect = t_enter < 0.0f;

    Vector3 normal = {0, 0, 0};
    if (intersect) {
        if (tx_x <= ty_x && tx_x <= tz_x) normal.x = v.x > 0.0f ? 1.0f : -1.0f;
        else if (ty_x <= tz_x)            normal.y = v.y > 0.0f ? 1.0f : -1.0f;
        else                              normal.z = v.z > 0.0f ? 1.0f : -1.0f;
    } else {
        if (tx_e >= ty_e && tx_e >= tz_e) normal.x = v.x > 0.0f ? -1.0f : 1.0f;
        else if (ty_e >= tz_e)            normal.y = v.y > 0.0f ? -1.0f : 1.0f;
        else                              normal.z = v.z > 0.0f ? -1.0f : 1.0f;
    }

    return {t, Vector3Add(body.center, Vector3Scale(v, t)), normal, intersect};
}
