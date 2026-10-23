#include "mc/pi.h"

void mc_pi_init(mc_pi_t *pi, float kp, float ki, float out_min, float out_max)
{
    pi->kp = kp;
    pi->ki = ki;
    pi->integral = 0.f;
    pi->out_min = out_min;
    pi->out_max = out_max;
}

void mc_pi_reset(mc_pi_t *pi)
{
    pi->integral = 0.f;
}

float mc_pi_step(mc_pi_t *pi, float error, float dt)
{
    float integ_new = pi->integral + (pi->ki * error * dt);
    float out = (pi->kp * error) + integ_new;

    if (out > pi->out_max) {
        out = pi->out_max;
        /* Conditional integration: do not wind up further into the stop.
         * Keep the integral that produced the clamp, and no more. */
        if (error > 0.f) {
            if (pi->integral > pi->out_max) {
                pi->integral = pi->out_max;
            }
        } else {
            pi->integral = integ_new;
        }
    } else if (out < pi->out_min) {
        out = pi->out_min;
        if (error < 0.f) {
            if (pi->integral < pi->out_min) {
                pi->integral = pi->out_min;
            }
        } else {
            pi->integral = integ_new;
        }
    } else {
        pi->integral = integ_new;
    }
    return out;
}
