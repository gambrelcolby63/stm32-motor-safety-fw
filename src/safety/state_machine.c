#include "mc/safety.h"

#include <string.h>

void mc_safety_init(mc_sm_t *sm)
{
    memset(sm, 0, sizeof(*sm));
    sm->state = MC_STATE_INIT;
    sm->act = MC_ACT_OFF;
    sm->fault = MC_FAULT_NONE;
}

static bool fault_condition_present(const mc_sm_in_t *in)
{
    return in->overcurrent || in->overtemperature || in->encoder_fault || in->stall ||
           in->limit_hit || in->keepout_violation || in->homing_failed;
}

static mc_fault_t pick_fault(const mc_sm_in_t *in)
{
    /* Fixed priority so a single latched code is deterministic. */
    if (in->overcurrent) {
        return MC_FAULT_OVERCURRENT;
    }
    if (in->encoder_fault) {
        return MC_FAULT_ENCODER;
    }
    if (in->overtemperature) {
        return MC_FAULT_OVERTEMP;
    }
    if (in->stall) {
        return MC_FAULT_STALL;
    }
    if (in->limit_hit) {
        return MC_FAULT_LIMIT;
    }
    if (in->keepout_violation) {
        return MC_FAULT_KEEPOUT;
    }
    if (in->homing_failed) {
        return MC_FAULT_HOMING;
    }
    return MC_FAULT_NONE;
}

static bool immediate_cutoff(mc_fault_t fault)
{
    return fault == MC_FAULT_OVERCURRENT || fault == MC_FAULT_ENCODER ||
           fault == MC_FAULT_WATCHDOG || fault == MC_FAULT_SELF_TEST;
}

static bool was_energized(mc_state_t state)
{
    return state == MC_STATE_ENABLED || state == MC_STATE_MOVING || state == MC_STATE_HOMING;
}

static void enter_fault(mc_sm_t *sm, mc_fault_t fault)
{
    bool energized = was_energized(sm->state);
    if (fault == MC_FAULT_ENCODER || fault == MC_FAULT_WATCHDOG) {
        sm->homed = false; /* SR-ENC-1: position reference is no longer trusted. */
    }
    sm->fault = fault;
    sm->state = MC_STATE_FAULT;
    sm->stopping = false;
    sm->stop_to_safe = false;
    if (!energized || immediate_cutoff(fault)) {
        /* SR-OC-1 / SR-ENC-1: drop the bridge on this tick. */
        sm->act = MC_ACT_OFF;
    } else {
        /* SR-BRK-1 / SR-OT-1 / SR-STALL-1 / SR-LIM-1: decelerate, then de-energize. */
        sm->act = MC_ACT_BRAKE;
    }
}

static void enter_estop(mc_sm_t *sm)
{
    /* SR-ESTOP-1 / SR-ESTOP-4 */
    sm->state = MC_STATE_ESTOP;
    sm->act = MC_ACT_OFF;
    sm->fault = MC_FAULT_ESTOP;
    sm->homed = false;
    sm->stopping = false;
    sm->stop_to_safe = false;
}

void mc_safety_step(mc_sm_t *sm, const mc_sm_in_t *in)
{
    if (in->estop) {
        if (sm->state != MC_STATE_ESTOP) {
            enter_estop(sm);
        } else {
            sm->act = MC_ACT_OFF;
        }
        return;
    }

    if (sm->state == MC_STATE_ESTOP) {
        sm->act = MC_ACT_OFF;
        /* SR-ESTOP-2: released input is not enough. SR-ESTOP-3: reset while
         * the input is still asserted never reaches this branch. */
        if (in->cmd_reset) {
            if (fault_condition_present(in)) {
                enter_fault(sm, pick_fault(in));
            } else {
                sm->state = MC_STATE_SAFE;
                sm->act = MC_ACT_OFF;
                sm->fault = MC_FAULT_NONE;
                sm->homed = false;
            }
        }
        return;
    }

    if (sm->state == MC_STATE_FAULT) {
        if (sm->act == MC_ACT_BRAKE && (in->braking_done || immediate_cutoff(sm->fault))) {
            sm->act = MC_ACT_OFF;
        }
        /* SR-RST-1 / SR-RST-2 */
        if (in->cmd_reset && !fault_condition_present(in)) {
            sm->state = MC_STATE_SAFE;
            sm->act = MC_ACT_OFF;
            sm->fault = MC_FAULT_NONE;
            sm->stopping = false;
            sm->stop_to_safe = false;
        }
        return;
    }

    if (sm->state == MC_STATE_INIT) {
        if (in->watchdog_reset) {
            enter_fault(sm, MC_FAULT_WATCHDOG); /* SR-WDG-1 */
            return;
        }
        if (!in->self_test_ok) {
            enter_fault(sm, MC_FAULT_SELF_TEST);
            return;
        }
        if (fault_condition_present(in)) {
            enter_fault(sm, pick_fault(in));
            return;
        }
        sm->state = MC_STATE_SAFE;
        sm->act = MC_ACT_OFF;
        sm->fault = MC_FAULT_NONE;
        sm->homed = false;
        return;
    }

    if (fault_condition_present(in)) {
        enter_fault(sm, pick_fault(in));
        return;
    }

    switch (sm->state) {
    case MC_STATE_SAFE:
        sm->act = MC_ACT_OFF;
        if (in->cmd_home) {
            sm->state = MC_STATE_HOMING;
            sm->act = MC_ACT_HOME;
            sm->homed = false;
            break;
        }
        /* SR-HOME-1: enable is refused until a homing sequence has completed.
         * A fresh heartbeat is required so an unattended link cannot enable. */
        if (in->cmd_enable && sm->homed && in->heartbeat_ok) {
            sm->state = MC_STATE_ENABLED;
            sm->act = MC_ACT_HOLD;
        }
        break;

    case MC_STATE_HOMING:
        /* SR-HB-1: losing the command link aborts homing with the bridge off. */
        if (!in->heartbeat_ok || in->cmd_disable || in->cmd_stop) {
            sm->state = MC_STATE_SAFE;
            sm->act = MC_ACT_OFF;
            break;
        }
        if (in->homing_done) {
            sm->homed = true; /* SR-HOME-2 */
            sm->state = MC_STATE_SAFE;
            sm->act = MC_ACT_OFF;
            break;
        }
        sm->act = MC_ACT_HOME;
        break;

    case MC_STATE_ENABLED:
        if (!in->heartbeat_ok || in->cmd_disable) {
            sm->state = MC_STATE_SAFE;
            sm->act = MC_ACT_OFF;
            break;
        }
        if (in->cmd_move) {
            /* SR-MOVE-1 */
            sm->state = MC_STATE_MOVING;
            sm->act = MC_ACT_MOVE;
            sm->stopping = false;
            sm->stop_to_safe = false;
            break;
        }
        sm->act = MC_ACT_HOLD;
        break;

    case MC_STATE_MOVING:
        if (!in->heartbeat_ok || in->cmd_disable) {
            sm->stopping = true;
            sm->stop_to_safe = true;
        } else if (in->cmd_stop) {
            sm->stopping = true;
            sm->stop_to_safe = false;
        }
        if (sm->stopping) {
            sm->act = MC_ACT_BRAKE;
            if (in->motion_done || in->braking_done) {
                if (sm->stop_to_safe || !in->heartbeat_ok) {
                    sm->state = MC_STATE_SAFE;
                    sm->act = MC_ACT_OFF;
                } else {
                    sm->state = MC_STATE_ENABLED;
                    sm->act = MC_ACT_HOLD;
                }
                sm->stopping = false;
                sm->stop_to_safe = false;
            }
            break;
        }
        if (in->motion_done) {
            sm->state = MC_STATE_ENABLED;
            sm->act = MC_ACT_HOLD;
            break;
        }
        sm->act = MC_ACT_MOVE;
        break;

    case MC_STATE_INIT:
    case MC_STATE_FAULT:
    case MC_STATE_ESTOP:
    default:
        sm->state = MC_STATE_FAULT;
        sm->act = MC_ACT_OFF;
        sm->fault = MC_FAULT_SELF_TEST;
        break;
    }
}
