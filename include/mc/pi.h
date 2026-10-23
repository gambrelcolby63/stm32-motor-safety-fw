#ifndef MC_PI_H
#define MC_PI_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float kp;
    float ki;
    float integral;
    float out_min;
    float out_max;
} mc_pi_t;

void mc_pi_init(mc_pi_t *pi, float kp, float ki, float out_min, float out_max);
void mc_pi_reset(mc_pi_t *pi);
float mc_pi_step(mc_pi_t *pi, float error, float dt);

#ifdef __cplusplus
}
#endif

#endif /* MC_PI_H */
