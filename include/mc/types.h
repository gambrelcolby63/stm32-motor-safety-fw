#ifndef MC_TYPES_H
#define MC_TYPES_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MC_AXIS_PAN = 0,
    MC_AXIS_TILT = 1
} mc_axis_id_t;

typedef enum {
    MC_STATE_INIT = 0,
    MC_STATE_SAFE,
    MC_STATE_HOMING,
    MC_STATE_ENABLED,
    MC_STATE_MOVING,
    MC_STATE_FAULT,
    MC_STATE_ESTOP
} mc_state_t;

/* What the power stage is allowed to do this tick. */
typedef enum {
    MC_ACT_OFF = 0,     /* bridge off, mechanical brake engaged */
    MC_ACT_BRAKE,       /* rate loop commanded to 0, then de-energize */
    MC_ACT_HOLD,        /* hold the captured position */
    MC_ACT_MOVE,        /* follow the trajectory or rate setpoint */
    MC_ACT_HOME         /* homing velocity */
} mc_act_t;

typedef enum {
    MC_FAULT_NONE = 0,
    MC_FAULT_SELF_TEST,
    MC_FAULT_ESTOP,
    MC_FAULT_OVERCURRENT,
    MC_FAULT_OVERTEMP,
    MC_FAULT_ENCODER,
    MC_FAULT_STALL,
    MC_FAULT_LIMIT,
    MC_FAULT_KEEPOUT,
    MC_FAULT_WATCHDOG,
    MC_FAULT_HOMING
} mc_fault_t;

const char *mc_state_name(mc_state_t state);
const char *mc_act_name(mc_act_t act);
const char *mc_fault_name(mc_fault_t fault);

#ifdef __cplusplus
}
#endif

#endif /* MC_TYPES_H */
