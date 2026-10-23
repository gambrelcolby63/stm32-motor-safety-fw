#include "mc/types.h"
#include "mc/homing.h"

const char *mc_state_name(mc_state_t state)
{
    switch (state) {
    case MC_STATE_INIT: return "INIT";
    case MC_STATE_SAFE: return "SAFE";
    case MC_STATE_HOMING: return "HOMING";
    case MC_STATE_ENABLED: return "ENABLED";
    case MC_STATE_MOVING: return "MOVING";
    case MC_STATE_FAULT: return "FAULT";
    case MC_STATE_ESTOP: return "ESTOP";
    default: return "UNKNOWN";
    }
}

const char *mc_act_name(mc_act_t act)
{
    switch (act) {
    case MC_ACT_OFF: return "OFF";
    case MC_ACT_BRAKE: return "BRAKE";
    case MC_ACT_HOLD: return "HOLD";
    case MC_ACT_MOVE: return "MOVE";
    case MC_ACT_HOME: return "HOME";
    default: return "UNKNOWN";
    }
}

const char *mc_fault_name(mc_fault_t fault)
{
    switch (fault) {
    case MC_FAULT_NONE: return "NONE";
    case MC_FAULT_SELF_TEST: return "SELF_TEST";
    case MC_FAULT_ESTOP: return "ESTOP";
    case MC_FAULT_OVERCURRENT: return "OVERCURRENT";
    case MC_FAULT_OVERTEMP: return "OVERTEMP";
    case MC_FAULT_ENCODER: return "ENCODER";
    case MC_FAULT_STALL: return "STALL";
    case MC_FAULT_LIMIT: return "LIMIT";
    case MC_FAULT_KEEPOUT: return "KEEPOUT";
    case MC_FAULT_WATCHDOG: return "WATCHDOG";
    case MC_FAULT_HOMING: return "HOMING";
    default: return "UNKNOWN";
    }
}

const char *mc_home_phase_name(mc_home_phase_t phase, int which)
{
    if (phase == MC_HOME_IDLE) {
        return "idle";
    }
    if (phase == MC_HOME_DONE) {
        return "done";
    }
    if (phase == MC_HOME_FAIL) {
        return "fail";
    }
    if (which == 1) {
        switch (phase) {
        case MC_HOME_SEEK: return "tilt-seek";
        case MC_HOME_BACKOFF: return "tilt-backoff";
        case MC_HOME_LEAVE: return "tilt-leave";
        case MC_HOME_INDEX: return "tilt-index";
        default: break;
        }
    }
    if (which == 2) {
        switch (phase) {
        case MC_HOME_SEEK: return "pan-seek";
        case MC_HOME_BACKOFF: return "pan-backoff";
        case MC_HOME_LEAVE: return "pan-leave";
        case MC_HOME_INDEX: return "pan-index";
        default: break;
        }
    }
    return "idle";
}
