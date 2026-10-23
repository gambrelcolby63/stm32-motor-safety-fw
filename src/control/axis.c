#include "mc/axis.h"

#include "mc/config.h"

void mc_axis_init(mc_axis_t *ax, float v_max, float a_max, float soft_min, float soft_max, bool limited)
{
    mc_pi_init(&ax->pos_pi, MC_POS_KP_DEFAULT, MC_POS_KI_DEFAULT, -v_max, v_max);
    mc_pi_init(&ax->rate_pi, MC_RATE_KP_DEFAULT, MC_RATE_KI_DEFAULT, -100.f, 100.f);
    mc_trap_init(&ax->trap, 0.f, v_max, a_max);
    ax->measured_deg = 0.f;
    ax->measured_rate_dps = 0.f;
    ax->hold_deg = 0.f;
    ax->v_max = v_max;
    ax->a_max = a_max;
    ax->soft_min = soft_min;
    ax->soft_max = soft_max;
    ax->limited = limited;
    ax->wrap = !limited;
    ax->mode = MC_AXIS_MODE_OFF;
    ax->duty = 0.f;
    ax->v_cmd = 0.f;
    ax->rate_setpoint = 0.f;
    ax->slew_v = 0.f;
}

void mc_axis_set_measurement(mc_axis_t *ax, float deg, float rate_dps)
{
    ax->measured_deg = deg;
    ax->measured_rate_dps = rate_dps;
}

void mc_axis_command_position(mc_axis_t *ax, float target_deg)
{
    float target = target_deg;
    if (ax->limited) {
        target = mc_clampf(target, ax->soft_min, ax->soft_max);
    } else {
        target = mc_wrap180(target);
    }
    /* Profile starts from the measured angle so the reference does not jump. */
    if (ax->mode != MC_AXIS_MODE_TRACK) {
        float start = ax->wrap ? mc_wrap180(ax->measured_deg) : ax->measured_deg;
        ax->trap.pos = start;
        ax->trap.vel = ax->measured_rate_dps;
        ax->slew_v = ax->measured_rate_dps;
    }
    mc_trap_set_target(&ax->trap, target);
    ax->trap.v_max = ax->v_max;
    ax->trap.a_max = ax->a_max;
    ax->mode = MC_AXIS_MODE_TRACK;
}

void mc_axis_command_rate(mc_axis_t *ax, float rate_dps)
{
    ax->rate_setpoint = mc_clampf(rate_dps, -ax->v_max, ax->v_max);
    ax->mode = MC_AXIS_MODE_RATE;
}

void mc_axis_hold_here(mc_axis_t *ax)
{
    ax->hold_deg = ax->measured_deg;
    ax->mode = MC_AXIS_MODE_HOLD;
    ax->slew_v = 0.f;
    mc_pi_reset(&ax->pos_pi);
}

void mc_axis_off(mc_axis_t *ax)
{
    ax->mode = MC_AXIS_MODE_OFF;
    ax->duty = 0.f;
    ax->v_cmd = 0.f;
    ax->slew_v = 0.f;
    ax->rate_setpoint = 0.f;
    mc_pi_reset(&ax->pos_pi);
    mc_pi_reset(&ax->rate_pi);
}

float mc_axis_step(mc_axis_t *ax, float dt)
{
    float v_ref;
    float target;
    float err;
    float duty;

    if (ax->mode == MC_AXIS_MODE_OFF) {
        mc_axis_off(ax);
        return 0.f;
    }

    if (ax->mode == MC_AXIS_MODE_RATE) {
        v_ref = mc_slew(ax->slew_v, ax->rate_setpoint, ax->a_max, dt);
        ax->slew_v = v_ref;
    } else {
        if (ax->mode == MC_AXIS_MODE_TRACK) {
            if (ax->limited) {
                ax->trap.target = mc_clampf(ax->trap.target, ax->soft_min, ax->soft_max);
            }
            mc_trap_step(&ax->trap, dt);
            target = ax->trap.pos;
        } else {
            target = ax->hold_deg;
        }
        if (ax->wrap) {
            err = mc_wrap180(target - ax->measured_deg);
        } else {
            err = target - ax->measured_deg;
        }
        v_ref = mc_pi_step(&ax->pos_pi, err, dt);
        v_ref = mc_clampf(v_ref, -ax->v_max, ax->v_max);
        v_ref = mc_slew(ax->slew_v, v_ref, ax->a_max, dt);
        ax->slew_v = v_ref;
    }

    if (ax->limited) {
        v_ref = mc_limit_velocity(ax->measured_deg, v_ref, ax->soft_min, ax->soft_max, ax->a_max);
    }
    ax->v_cmd = v_ref;

    duty = mc_pi_step(&ax->rate_pi, v_ref - ax->measured_rate_dps, dt);
    duty = mc_clampf(duty, -100.f, 100.f);
    ax->duty = duty;
    return duty;
}
