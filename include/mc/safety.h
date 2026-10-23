#ifndef MC_SAFETY_H
#define MC_SAFETY_H

#include "mc/types.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    mc_state_t state;
    mc_act_t act;
    mc_fault_t fault;
    bool homed;
    bool stopping;
    bool stop_to_safe;
} mc_sm_t;

typedef struct {
    bool self_test_ok;
    bool estop;
    bool overcurrent;
    bool overtemperature;
    bool encoder_fault;
    bool stall;
    bool limit_hit;
    bool keepout_violation;
    bool watchdog_reset;
    bool homing_failed;
    bool homing_done;
    bool cmd_home;
    bool cmd_enable;
    bool cmd_disable;
    bool cmd_move;
    bool cmd_stop;
    bool cmd_reset;
    bool motion_done;
    bool braking_done;
    bool heartbeat_ok;
} mc_sm_in_t;

void mc_safety_init(mc_sm_t *sm);
void mc_safety_step(mc_sm_t *sm, const mc_sm_in_t *in);

#ifdef __cplusplus
}
#endif

#endif /* MC_SAFETY_H */
