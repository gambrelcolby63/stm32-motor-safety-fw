#include "mc/safety.h"
#include "mc/trace.h"

#include <gtest/gtest.h>

namespace {

mc_sm_in_t ready_input()
{
    mc_sm_in_t in{};
    in.self_test_ok = true;
    in.heartbeat_ok = true;
    return in;
}

mc_sm_t step_from(mc_state_t state, bool homed, mc_act_t act, mc_fault_t fault, const mc_sm_in_t &in)
{
    mc_sm_t sm{};
    mc_safety_init(&sm);
    sm.state = state;
    sm.homed = homed;
    sm.act = act;
    sm.fault = fault;
    mc_safety_step(&sm, &in);
    return sm;
}

}  // namespace

TEST(Safety, InitOutcomes)
{
    mc_sm_in_t in = ready_input();
    mc_sm_t sm;

    sm = step_from(MC_STATE_INIT, false, MC_ACT_OFF, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.state, MC_STATE_SAFE);
    EXPECT_EQ(sm.act, MC_ACT_OFF);
    EXPECT_FALSE(sm.homed);

    in.self_test_ok = false;
    sm = step_from(MC_STATE_INIT, false, MC_ACT_OFF, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.state, MC_STATE_FAULT);
    EXPECT_EQ(sm.fault, MC_FAULT_SELF_TEST);
    EXPECT_EQ(sm.act, MC_ACT_OFF);

    in = ready_input();
    in.watchdog_reset = true;
    sm = step_from(MC_STATE_INIT, false, MC_ACT_OFF, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.state, MC_STATE_FAULT);
    EXPECT_EQ(sm.fault, MC_FAULT_WATCHDOG);
    EXPECT_FALSE(sm.homed);
    mc_trace_hit("SR-WDG-1");

    in = ready_input();
    in.estop = true;
    in.self_test_ok = false;
    sm = step_from(MC_STATE_INIT, false, MC_ACT_OFF, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.state, MC_STATE_ESTOP);
    EXPECT_EQ(sm.act, MC_ACT_OFF);
}

TEST(Safety, EstopPreemptsAndLatches)
{
    mc_sm_in_t in = ready_input();
    mc_sm_t sm;
    in.estop = true;

    sm = step_from(MC_STATE_MOVING, true, MC_ACT_MOVE, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.state, MC_STATE_ESTOP);
    EXPECT_EQ(sm.act, MC_ACT_OFF);
    EXPECT_FALSE(sm.homed);
    sm = step_from(MC_STATE_ENABLED, true, MC_ACT_HOLD, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.state, MC_STATE_ESTOP);
    EXPECT_EQ(sm.act, MC_ACT_OFF);
    sm = step_from(MC_STATE_HOMING, false, MC_ACT_HOME, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.state, MC_STATE_ESTOP);
    EXPECT_EQ(sm.act, MC_ACT_OFF);
    sm = step_from(MC_STATE_SAFE, true, MC_ACT_OFF, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.state, MC_STATE_ESTOP);
    mc_trace_hit("SR-ESTOP-1");
    mc_trace_hit("SR-ESTOP-4");

    in.estop = false;
    sm = step_from(MC_STATE_ESTOP, false, MC_ACT_OFF, MC_FAULT_ESTOP, in);
    EXPECT_EQ(sm.state, MC_STATE_ESTOP);
    EXPECT_EQ(sm.act, MC_ACT_OFF);
    mc_trace_hit("SR-ESTOP-2");

    in.estop = true;
    in.cmd_reset = true;
    sm = step_from(MC_STATE_ESTOP, false, MC_ACT_OFF, MC_FAULT_ESTOP, in);
    EXPECT_EQ(sm.state, MC_STATE_ESTOP);
    mc_trace_hit("SR-ESTOP-3");

    in.estop = false;
    sm = step_from(MC_STATE_ESTOP, false, MC_ACT_OFF, MC_FAULT_ESTOP, in);
    EXPECT_EQ(sm.state, MC_STATE_SAFE);
    EXPECT_EQ(sm.act, MC_ACT_OFF);
    EXPECT_FALSE(sm.homed);
    EXPECT_EQ(sm.fault, MC_FAULT_NONE);
}

TEST(Safety, OvercurrentDropsTheBridgeImmediately)
{
    mc_sm_in_t in = ready_input();
    mc_sm_t sm;
    in.overcurrent = true;
    sm = step_from(MC_STATE_MOVING, true, MC_ACT_MOVE, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.state, MC_STATE_FAULT);
    EXPECT_EQ(sm.fault, MC_FAULT_OVERCURRENT);
    EXPECT_EQ(sm.act, MC_ACT_OFF);
    mc_trace_hit("SR-OC-1");
}

TEST(Safety, OvertempStallLimitAndKeepoutBrake)
{
    mc_sm_in_t in = ready_input();
    mc_sm_t sm;

    in.overtemperature = true;
    sm = step_from(MC_STATE_MOVING, true, MC_ACT_MOVE, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.state, MC_STATE_FAULT);
    EXPECT_EQ(sm.fault, MC_FAULT_OVERTEMP);
    EXPECT_EQ(sm.act, MC_ACT_BRAKE);
    EXPECT_TRUE(sm.homed);
    mc_trace_hit("SR-OT-1");
    mc_trace_hit("SR-BRK-1");

    in = ready_input();
    in.braking_done = true;
    sm = step_from(MC_STATE_FAULT, true, MC_ACT_BRAKE, MC_FAULT_OVERTEMP, in);
    EXPECT_EQ(sm.state, MC_STATE_FAULT);
    EXPECT_EQ(sm.act, MC_ACT_OFF);

    in = ready_input();
    in.stall = true;
    sm = step_from(MC_STATE_MOVING, true, MC_ACT_MOVE, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.fault, MC_FAULT_STALL);
    EXPECT_EQ(sm.act, MC_ACT_BRAKE);
    mc_trace_hit("SR-STALL-1");

    in = ready_input();
    in.limit_hit = true;
    sm = step_from(MC_STATE_ENABLED, true, MC_ACT_HOLD, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.fault, MC_FAULT_LIMIT);
    EXPECT_EQ(sm.act, MC_ACT_BRAKE);
    mc_trace_hit("SR-LIM-1");

    in = ready_input();
    in.keepout_violation = true;
    sm = step_from(MC_STATE_MOVING, true, MC_ACT_MOVE, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.fault, MC_FAULT_KEEPOUT);
    EXPECT_EQ(sm.act, MC_ACT_BRAKE);
    mc_trace_hit("SR-KO-2");
}

TEST(Safety, EncoderFaultClearsTheReference)
{
    mc_sm_in_t in = ready_input();
    mc_sm_t sm;
    in.encoder_fault = true;
    sm = step_from(MC_STATE_ENABLED, true, MC_ACT_HOLD, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.fault, MC_FAULT_ENCODER);
    EXPECT_EQ(sm.act, MC_ACT_OFF);
    EXPECT_FALSE(sm.homed);
    mc_trace_hit("SR-ENC-1");
}

TEST(Safety, ResetRequiresAClearCondition)
{
    mc_sm_in_t in = ready_input();
    mc_sm_t sm;
    in.overtemperature = true;
    in.cmd_reset = true;
    sm = step_from(MC_STATE_FAULT, true, MC_ACT_OFF, MC_FAULT_OVERTEMP, in);
    EXPECT_EQ(sm.state, MC_STATE_FAULT);
    mc_trace_hit("SR-RST-1");

    in.overtemperature = false;
    sm = step_from(MC_STATE_FAULT, true, MC_ACT_OFF, MC_FAULT_OVERTEMP, in);
    EXPECT_EQ(sm.state, MC_STATE_SAFE);
    EXPECT_EQ(sm.act, MC_ACT_OFF);
    EXPECT_EQ(sm.fault, MC_FAULT_NONE);
    mc_trace_hit("SR-RST-2");
}

TEST(Safety, EnableAndMoveRequireHomeThenEnable)
{
    mc_sm_in_t in = ready_input();
    mc_sm_t sm;
    in.cmd_enable = true;
    sm = step_from(MC_STATE_SAFE, false, MC_ACT_OFF, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.state, MC_STATE_SAFE);
    EXPECT_EQ(sm.act, MC_ACT_OFF);
    mc_trace_hit("SR-HOME-1");

    in.cmd_move = true;
    in.cmd_enable = false;
    sm = step_from(MC_STATE_SAFE, true, MC_ACT_OFF, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.state, MC_STATE_SAFE);

    in.cmd_move = false;
    in.cmd_enable = true;
    sm = step_from(MC_STATE_SAFE, true, MC_ACT_OFF, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.state, MC_STATE_ENABLED);
    EXPECT_EQ(sm.act, MC_ACT_HOLD);

    in.cmd_enable = false;
    in.cmd_move = true;
    sm = step_from(MC_STATE_ENABLED, true, MC_ACT_HOLD, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.state, MC_STATE_MOVING);
    EXPECT_EQ(sm.act, MC_ACT_MOVE);
    mc_trace_hit("SR-MOVE-1");

    in.cmd_move = false;
    in.cmd_enable = true;
    sm = step_from(MC_STATE_FAULT, true, MC_ACT_OFF, MC_FAULT_OVERTEMP, in);
    EXPECT_EQ(sm.state, MC_STATE_FAULT);
    sm = step_from(MC_STATE_ESTOP, false, MC_ACT_OFF, MC_FAULT_ESTOP, in);
    EXPECT_EQ(sm.state, MC_STATE_ESTOP);
}

TEST(Safety, HomingAndHeartbeat)
{
    mc_sm_in_t in = ready_input();
    mc_sm_t sm;
    in.cmd_home = true;
    sm = step_from(MC_STATE_SAFE, true, MC_ACT_OFF, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.state, MC_STATE_HOMING);
    EXPECT_EQ(sm.act, MC_ACT_HOME);
    EXPECT_FALSE(sm.homed);

    in.cmd_home = false;
    in.homing_done = true;
    sm = step_from(MC_STATE_HOMING, false, MC_ACT_HOME, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.state, MC_STATE_SAFE);
    EXPECT_TRUE(sm.homed);
    EXPECT_EQ(sm.act, MC_ACT_OFF);

    in.homing_done = false;
    in.homing_failed = true;
    sm = step_from(MC_STATE_HOMING, false, MC_ACT_HOME, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.state, MC_STATE_FAULT);
    EXPECT_EQ(sm.fault, MC_FAULT_HOMING);

    in = ready_input();
    in.heartbeat_ok = false;
    sm = step_from(MC_STATE_MOVING, true, MC_ACT_MOVE, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.state, MC_STATE_MOVING);
    EXPECT_EQ(sm.act, MC_ACT_BRAKE);
    EXPECT_TRUE(sm.stopping);

    in.braking_done = true;
    sm = step_from(MC_STATE_MOVING, true, MC_ACT_BRAKE, MC_FAULT_NONE, in);
    /* The helper does not copy the internal stopping flag. Drive the real object. */
    {
        mc_sm_t live;
        mc_sm_in_t loss = ready_input();
        mc_safety_init(&live);
        live.state = MC_STATE_MOVING;
        live.act = MC_ACT_MOVE;
        live.homed = true;
        loss.heartbeat_ok = false;
        mc_safety_step(&live, &loss);
        EXPECT_EQ(live.act, MC_ACT_BRAKE);
        loss.braking_done = true;
        mc_safety_step(&live, &loss);
        EXPECT_EQ(live.state, MC_STATE_SAFE);
        EXPECT_EQ(live.act, MC_ACT_OFF);
        EXPECT_EQ(live.fault, MC_FAULT_NONE);
    }
    mc_trace_hit("SR-HB-1");

    in = ready_input();
    in.heartbeat_ok = false;
    sm = step_from(MC_STATE_ENABLED, true, MC_ACT_HOLD, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.state, MC_STATE_SAFE);
    EXPECT_EQ(sm.act, MC_ACT_OFF);

    in = ready_input();
    in.cmd_stop = true;
    {
        mc_sm_t live;
        mc_safety_init(&live);
        live.state = MC_STATE_MOVING;
        live.act = MC_ACT_MOVE;
        live.homed = true;
        mc_safety_step(&live, &in);
        EXPECT_EQ(live.state, MC_STATE_MOVING);
        EXPECT_EQ(live.act, MC_ACT_BRAKE);
        in.braking_done = true;
        in.cmd_stop = false;
        mc_safety_step(&live, &in);
        EXPECT_EQ(live.state, MC_STATE_ENABLED);
        EXPECT_EQ(live.act, MC_ACT_HOLD);
    }

    in = ready_input();
    in.motion_done = true;
    sm = step_from(MC_STATE_MOVING, true, MC_ACT_MOVE, MC_FAULT_NONE, in);
    EXPECT_EQ(sm.state, MC_STATE_ENABLED);
    EXPECT_EQ(sm.act, MC_ACT_HOLD);
}

TEST(Safety, EstopResetStillSeesAnotherFault)
{
    mc_sm_in_t in = ready_input();
    mc_sm_t sm;
    in.cmd_reset = true;
    in.overtemperature = true;
    sm = step_from(MC_STATE_ESTOP, false, MC_ACT_OFF, MC_FAULT_ESTOP, in);
    EXPECT_EQ(sm.state, MC_STATE_FAULT);
    EXPECT_EQ(sm.fault, MC_FAULT_OVERTEMP);
    EXPECT_EQ(sm.act, MC_ACT_OFF);
}
