#include "mc/keepout.h"

#include <string.h>

namespace {

bool point_in_zone(const mc_zone_t &z, float pan, float tilt)
{
    return pan >= z.pan_min && pan <= z.pan_max && tilt >= z.tilt_min && tilt <= z.tilt_max;
}

/* Liang-Barsky clip: true when the segment intersects the inclusive rectangle. */
bool segment_hits_rect(float x0, float y0, float x1, float y1,
                       float xmin, float xmax, float ymin, float ymax)
{
    auto clip = [](float p, float q, float &t0, float &t1) -> bool {
        if (p == 0.f) {
            return q >= 0.f;
        }
        float r = q / p;
        if (p < 0.f) {
            if (r > t1) {
                return false;
            }
            if (r > t0) {
                t0 = r;
            }
        } else {
            if (r < t0) {
                return false;
            }
            if (r < t1) {
                t1 = r;
            }
        }
        return true;
    };

    float t0 = 0.f;
    float t1 = 1.f;
    float dx = x1 - x0;
    float dy = y1 - y0;
    if (!clip(-dx, x0 - xmin, t0, t1)) {
        return false;
    }
    if (!clip(dx, xmax - x0, t0, t1)) {
        return false;
    }
    if (!clip(-dy, y0 - ymin, t0, t1)) {
        return false;
    }
    if (!clip(dy, ymax - y0, t0, t1)) {
        return false;
    }
    return true;
}

bool segment_hits_table(const mc_keepout_t *k, float pan0, float tilt0, float pan1, float tilt1)
{
    for (int i = 0; i < MC_ZONE_MAX; ++i) {
        const mc_zone_t &z = k->zones[i];
        if (!z.used) {
            continue;
        }
        if (segment_hits_rect(pan0, tilt0, pan1, tilt1, z.pan_min, z.pan_max, z.tilt_min, z.tilt_max)) {
            return true;
        }
    }
    return false;
}

float wrap180(float deg)
{
    while (deg >= 180.f) {
        deg -= 360.f;
    }
    while (deg < -180.f) {
        deg += 360.f;
    }
    return deg;
}

}  // namespace

extern "C" void mc_keepout_clear(mc_keepout_t *k)
{
    memset(k, 0, sizeof(*k));
}

extern "C" bool mc_keepout_add(mc_keepout_t *k, float pan_min, float pan_max, float tilt_min, float tilt_max)
{
    if (pan_min > pan_max || tilt_min > tilt_max) {
        return false;
    }
    for (int i = 0; i < MC_ZONE_MAX; ++i) {
        if (!k->zones[i].used) {
            k->zones[i].pan_min = pan_min;
            k->zones[i].pan_max = pan_max;
            k->zones[i].tilt_min = tilt_min;
            k->zones[i].tilt_max = tilt_max;
            k->zones[i].used = true;
            return true;
        }
    }
    return false;
}

extern "C" bool mc_keepout_contains(const mc_keepout_t *k, float pan, float tilt)
{
    for (int i = 0; i < MC_ZONE_MAX; ++i) {
        if (k->zones[i].used && point_in_zone(k->zones[i], pan, tilt)) {
            return true;
        }
    }
    return false;
}

extern "C" bool mc_keepout_segment_hits(const mc_keepout_t *k,
                                       float pan0, float tilt0,
                                       float pan1, float tilt1,
                                       bool pan_wrap)
{
    if (!pan_wrap) {
        return segment_hits_table(k, pan0, tilt0, pan1, tilt1);
    }
    float a0 = wrap180(pan0);
    float a1 = wrap180(pan1);
    float delta = wrap180(a1 - a0);
    /* Shortest path stays on one side of the branch cut. */
    if (a0 + delta >= -180.f && a0 + delta < 180.f) {
        return segment_hits_table(k, a0, tilt0, a0 + delta, tilt1);
    }
    /* Split at ±180 so each piece is continuous in unwrapped space. */
    if (delta >= 0.f) {
        float t_split = (180.f - a0) / delta;
        float tilt_split = tilt0 + ((tilt1 - tilt0) * t_split);
        return segment_hits_table(k, a0, tilt0, 180.f, tilt_split) ||
               segment_hits_table(k, -180.f, tilt_split, a0 + delta - 360.f, tilt1);
    }
    float t_split = (-180.f - a0) / delta;
    float tilt_split = tilt0 + ((tilt1 - tilt0) * t_split);
    return segment_hits_table(k, a0, tilt0, -180.f, tilt_split) ||
           segment_hits_table(k, 180.f, tilt_split, a0 + delta + 360.f, tilt1);
}
