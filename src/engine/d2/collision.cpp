#include "collision.hpp"

#include <cmath>

namespace engine::d2 {
namespace {

constexpr float INF = 1e30f;

bool in_range(float v, float lo, float hi) { return v > lo && v < hi; }

struct Face {
    Vec2f n;
    float p, q;
};

void liang_barsky(Vec2f v, const Vec2f& mn, const Vec2f& mx, float& t1, Vec2f& n1, float& t2, Vec2f& n2) {
    t1 = -INF;
    t2 = INF;
    n1 = {};
    n2 = {};

    Face faces[] = {
        {{-1, 0}, -v.x, -mn.x},
        {{1, 0}, v.x, mx.x},
        {{0, -1}, -v.y, -mn.y},
        {{0, 1}, v.y, mx.y},
    };

    for (auto& f : faces) {
        if (f.p == 0) {
            if (f.q <= 0) {
                t1 = 1;
                t2 = -1;
                return;
            }
        } else {
            float r = f.q / f.p;
            if (f.p < 0) {
                if (r > t2) {
                    t1 = 1;
                    t2 = -1;
                    return;
                }
                if (r > t1) {
                    t1 = r;
                    n1 = f.n;
                }
            } else {
                if (r < t1) {
                    t1 = 1;
                    t2 = -1;
                    return;
                }
                if (r < t2) {
                    t2 = r;
                    n2 = f.n;
                }
            }
        }
    }
}

} // namespace

SweepResult2 sweep(const Box2& body, Vec2f target, const Box2& other) {
    Vec2f v = target - body.center;

    Vec2f mn{
        other.center.x - other.halfX - body.center.x - body.halfX,
        other.center.y - other.halfY - body.center.y - body.halfY,
    };
    Vec2f mx{
        other.center.x + other.halfX - body.center.x + body.halfX,
        other.center.y + other.halfY - body.center.y + body.halfY,
    };

    bool intersect = in_range(0, mn.x, mx.x) && in_range(0, mn.y, mx.y);

    if (intersect && v.x == 0 && v.y == 0) {
        float px = std::fabs(mn.x) < std::fabs(mx.x) ? mn.x : mx.x;
        float py = std::fabs(mn.y) < std::fabs(mx.y) ? mn.y : mx.y;
        float ax = std::fabs(px), ay = std::fabs(py);

        Vec2f normal{};
        if (ax <= ay) {
            normal.x = px > 0 ? 1 : -1;
        } else {
            normal.y = py > 0 ? 1 : -1;
        }

        return {0, body.center, normal, true};
    }

    float t1, t2;
    Vec2f n1, n2;
    liang_barsky(v, mn, mx, t1, n1, t2, n2);
    if (t1 > t2) {
        return {1, target, {}, false};
    }

    float t;
    Vec2f normal;
    if (intersect) {
        if (std::fabs(t1) <= std::fabs(t2)) {
            t = t1;
            normal = n1;
        } else {
            t = t2;
            normal = n2;
        }
    } else if (t1 >= 0 && t1 <= 1) {
        t = t1;
        normal = n1;
    } else {
        return {1, target, {}, false};
    }

    Vec2f os{other.halfX, other.halfY};
    Vec2f bs{body.halfX, body.halfY};
    Vec2f pos;
    pos.x = normal.x < 0   ? other.center.x - os.x - bs.x
            : normal.x > 0 ? other.center.x + os.x + bs.x
                           : body.center.x + v.x * t;
    pos.y = normal.y < 0   ? other.center.y - os.y - bs.y
            : normal.y > 0 ? other.center.y + os.y + bs.y
                           : body.center.y + v.y * t;

    return {t, pos, normal, intersect};
}

} // namespace engine::d2
