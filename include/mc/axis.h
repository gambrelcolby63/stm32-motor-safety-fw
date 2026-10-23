#ifndef MC_AXIS_H
#define MC_AXIS_H

#include "mc/pi.h"
#include "mc/trajectory.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MC_AXIS_MODE_OFF = 0,
    MC_AXIS_MODE_HOLD,
    MC_AXIS_MODE_TRACK,
    MC_AXIS_MODE_RATE
} mc_axis_mode_t;

typedef struct {
    mc_pi_t pos_pi;
    mc_pi_t rate_pi;
    mc_trap_t trap;
    float measured_deg;
    float measured_rate_dps;
    float hold_deg;
    float v_max;
    float a_max;
    float soft_min;
    float soft_max;
    bool limited;
    bool wrap;
    mc_axis_mode_t mode;
    float duty;
    float v_cmd;
    float rate_setpoint;
    float slew_v;
} mc_axis_t;

void mc_axis_init(mc_axis_t *ax, float v_max, float a_max, float soft_min, float soft_max, bool limited);
void mc_axis_set_measurement(mc_axis_t *ax, float deg, float rate_dps);
void mc_axis_command_position(mc_axis_t *ax, float target_deg);
void mc_axis_command_rate(mc_axis_t *ax, float rate_dps);
void mc_axis_hold_here(mc_axis_t *ax);
void mc_axis_off(mc_axis_t *ax);
float mc_axis_step(mc_axis_t *ax, float dt);

#ifdef __cplusplus
}
#endif

#endif /* MC_AXIS_H */
