#include "mc/trajectory.h"

#include <math.h>

void mc_trap_init(mc_trap_t *t, float pos, float v_max, float a_max)
{
    t->pos = pos;
    t->vel = 0.f;
    t->target = pos;
    t->v_max = v_max;
    t->a_max = a_max;
    t->pos_tol = 0.05f;
    t->vel_tol = 0.5f;
    t->done = true;
}

void mc_trap_set_target(mc_trap_t *t, float target)
{
    t->target = target;
    t->done = false;
}

float mc_clampf(float v, float lo, float hi)
{
    if (v < lo) {
        return lo;
    }
    if (v > hi) {
        return hi;
    }
    return v;
}

float mc_slew(float current, float target, float a_max, float dt)
{
    float max_step = a_max * dt;
    float delta = target - current;
    if (delta > max_step) {
        delta = max_step;
    } else if (delta < -max_step) {
        delta = -max_step;
    }
    return current + delta;
}

float mc_limit_velocity(float pos, float v_cmd, float pos_min, float pos_max, float a_max)
{
    float room_hi;
    float room_lo;
    float v_hi;
    float v_lo;
    if (a_max < 1.f) {
        a_max = 1.f;
    }
    room_hi = pos_max - pos;
    room_lo = pos - pos_min;
    if (room_hi < 0.f) {
        room_hi = 0.f;
    }
    if (room_lo < 0.f) {
        room_lo = 0.f;
    }
    /* Stop exactly at the soft limit: v <= sqrt(2 a s). SR-LIM-2. */
    v_hi = sqrtf(2.f * a_max * room_hi);
    v_lo = sqrtf(2.f * a_max * room_lo);
    if (v_cmd > v_hi) {
        v_cmd = v_hi;
    }
    if (v_cmd < -v_lo) {
        v_cmd = -v_lo;
    }
    return v_cmd;
}

float mc_wrap180(float deg)
{
    float x;
    if (deg >= -180.f && deg < 180.f) {
        return deg;
    }
    x = fmodf(deg + 180.f, 360.f);
    if (x < 0.f) {
        x += 360.f;
    }
    return x - 180.f;
}

void mc_trap_step(mc_trap_t *t, float dt)
{
    float err;
    float stopping;
    float v_des;
    float max_dv;
    float dv;

    if (t->a_max < 1.f) {
        t->a_max = 1.f;
    }
    if (t->v_max < 1.f) {
        t->v_max = 1.f;
    }
    if (dt <= 0.f) {
        return;
    }

    err = t->target - t->pos;
    max_dv = t->a_max * dt;
    /* Arrive inside the accel budget: zeroing this tick must not exceed a_max. */
    if ((fabsf(err) <= t->pos_tol) && (fabsf(t->vel) <= max_dv)) {
        t->pos = t->target;
        t->vel = 0.f;
        t->done = true;
        return;
    }
    /* Speed that can still stop in the remaining distance, then slew to it. */
    stopping = sqrtf(2.f * t->a_max * fabsf(err));
    if (stopping > t->v_max) {
        stopping = t->v_max;
    }
    v_des = copysignf(stopping, err);
    dv = v_des - t->vel;
    if (dv > max_dv) {
        dv = max_dv;
    } else if (dv < -max_dv) {
        dv = -max_dv;
    }
    t->vel += dv;
    t->pos += t->vel * dt;
    t->done = false;
}
