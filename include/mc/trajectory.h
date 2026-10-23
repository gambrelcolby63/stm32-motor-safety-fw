#ifndef MC_TRAJECTORY_H
#define MC_TRAJECTORY_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float pos;
    float vel;
    float target;
    float v_max;
    float a_max;
    float pos_tol;
    float vel_tol;
    bool done;
} mc_trap_t;

void mc_trap_init(mc_trap_t *t, float pos, float v_max, float a_max);
void mc_trap_set_target(mc_trap_t *t, float target);
void mc_trap_step(mc_trap_t *t, float dt);

float mc_slew(float current, float target, float a_max, float dt);
float mc_limit_velocity(float pos, float v_cmd, float pos_min, float pos_max, float a_max);
float mc_wrap180(float deg);
float mc_clampf(float v, float lo, float hi);

#ifdef __cplusplus
}
#endif

#endif /* MC_TRAJECTORY_H */
