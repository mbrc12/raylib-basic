#include "collision.hpp"
#include <cmath>

namespace {

constexpr float INF = 1e30f;

bool in_range(float v, float lo, float hi) { return v > lo && v < hi; }

struct Face {
    Vec3  n;
    float p, q;
};

void liang_barsky(Vec3 v, const Vec3& mn, const Vec3& mx,
                  float& t1, Vec3& n1, float& t2, Vec3& n2) {
    t1 = -INF; t2 = INF;
    n1 = {};   n2 = {};

    Face faces[] = {
        {{-1, 0, 0}, -v.x, -mn.x}, // -x (min x)
        {{ 1, 0, 0},  v.x,  mx.x}, // +x (max x)
        {{ 0,-1, 0}, -v.y, -mn.y}, // -y (min y)
        {{ 0, 1, 0},  v.y,  mx.y}, // +y (max y)
        {{ 0, 0,-1}, -v.z, -mn.z}, // -z (min z)
        {{ 0, 0, 1},  v.z,  mx.z}, // +z (max z)
    };

    for (auto& f : faces) {
        if (f.p == 0) {
            if (f.q <= 0) { t1 = 1; t2 = -1; return; }
        } else {
            float r = f.q / f.p;
            if (f.p < 0) {
                if (r > t2) { t1 = 1; t2 = -1; return; }
                if (r > t1) { t1 = r; n1 = f.n; }
            } else {
                if (r < t1) { t1 = 1; t2 = -1; return; }
                if (r < t2) { t2 = r; n2 = f.n; }
            }
        }
    }
}

} // namespace

SweepResult sweep(const Box& body, Vec3 target, const Box& other) {
    Vec3 v = target - body.center;

    Vec3 mn{
        other.center.x - other.halfX - body.center.x - body.halfX,
        other.center.y - other.halfY - body.center.y - body.halfY,
        other.center.z - other.halfZ - body.center.z - body.halfZ,
    };
    Vec3 mx{
        other.center.x + other.halfX - body.center.x + body.halfX,
        other.center.y + other.halfY - body.center.y + body.halfY,
        other.center.z + other.halfZ - body.center.z + body.halfZ,
    };

    bool intersect = in_range(0, mn.x, mx.x) &&
                     in_range(0, mn.y, mx.y) &&
                     in_range(0, mn.z, mx.z);

    if (intersect && v.x == 0 && v.y == 0 && v.z == 0) {
        float px = fabsf(mn.x) < fabsf(mx.x) ? mn.x : mx.x;
        float py = fabsf(mn.y) < fabsf(mx.y) ? mn.y : mx.y;
        float pz = fabsf(mn.z) < fabsf(mx.z) ? mn.z : mx.z;
        float ax = fabsf(px), ay = fabsf(py), az = fabsf(pz);

        Vec3 normal{};
        if (ax <= ay && ax <= az)      normal.x = px > 0 ? 1 : -1;
        else if (ay <= az)             normal.y = py > 0 ? 1 : -1;
        else                           normal.z = pz > 0 ? 1 : -1;
        return {0, body.center, normal, true};
    }

    float t1, t2;
    Vec3 n1, n2;
    liang_barsky(v, mn, mx, t1, n1, t2, n2);
    if (t1 > t2) return {1, target, {}, false};

    float t;
    Vec3 normal;
    if (intersect) {
        if (fabsf(t1) <= fabsf(t2)) { t = t1; normal = n1; }
        else                        { t = t2; normal = n2; }
    } else if (t1 >= 0 && t1 <= 1) {
        t = t1; normal = n1;
    } else {
        return {1, target, {}, false};
    }

    Vec3 os{other.halfX, other.halfY, other.halfZ};
    Vec3 bs{body.halfX, body.halfY, body.halfZ};
    Vec3 pos;
    pos.x = normal.x < 0 ? other.center.x - os.x - bs.x
          : normal.x > 0 ? other.center.x + os.x + bs.x
          : body.center.x + v.x * t;
    pos.y = normal.y < 0 ? other.center.y - os.y - bs.y
          : normal.y > 0 ? other.center.y + os.y + bs.y
          : body.center.y + v.y * t;
    pos.z = normal.z < 0 ? other.center.z - os.z - bs.z
          : normal.z > 0 ? other.center.z + os.z + bs.z
          : body.center.z + v.z * t;

    return {t, pos, normal, intersect};
}
