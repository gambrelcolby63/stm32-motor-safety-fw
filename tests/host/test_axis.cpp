#include "mc/axis.h"
#include "mc/trace.h"

#include <gtest/gtest.h>

TEST(Axis, PanCanBeContinuousOrLimitedTiltPatternIsLimited)
{
    mc_axis_t pan;
    mc_axis_t tilt;
    mc_axis_init(&pan, 120.f, 360.f, -170.f, 170.f, true);
    mc_axis_init(&tilt, 80.f, 280.f, -45.f, 90.f, true);
    EXPECT_TRUE(pan.limited);
    EXPECT_FALSE(pan.wrap);
    EXPECT_TRUE(tilt.limited);
    EXPECT_FALSE(tilt.wrap);

    mc_axis_init(&pan, 120.f, 360.f, -170.f, 170.f, false);
    EXPECT_FALSE(pan.limited);
    EXPECT_TRUE(pan.wrap);
    mc_trace_hit("SR-PAN-1");
}

TEST(Axis, PositionLoopCommandsABoundedRate)
{
    mc_axis_t axis;
    float duty;
    mc_axis_init(&axis, 40.f, 80.f, -100.f, 100.f, true);
    axis.pos_pi.ki = 0.f;
    axis.rate_pi.ki = 0.f;
    mc_axis_set_measurement(&axis, 0.f, 0.f);
    mc_axis_command_position(&axis, 20.f);
    duty = mc_axis_step(&axis, 0.001f);
    EXPECT_GT(axis.v_cmd, 0.f);
    EXPECT_LE(axis.v_cmd, 40.f);
    EXPECT_GT(duty, 0.f);
    EXPECT_LE(duty, 100.f);
}

TEST(Axis, SoftLimitCutsVelocityAtTheStop)
{
    mc_axis_t axis;
    mc_axis_init(&axis, 100.f, 50.f, -10.f, 10.f, true);
    axis.pos_pi.kp = 10.f;
    axis.pos_pi.ki = 0.f;
    axis.rate_pi.ki = 0.f;
    mc_axis_set_measurement(&axis, 10.f, 0.f);
    mc_axis_command_rate(&axis, 80.f);
    (void)mc_axis_step(&axis, 0.001f);
    EXPECT_NEAR(axis.v_cmd, 0.f, 1e-3);
}

TEST(Axis, OffClearsDuty)
{
    mc_axis_t axis;
    mc_axis_init(&axis, 40.f, 80.f, -10.f, 10.f, true);
    mc_axis_set_measurement(&axis, 0.f, 0.f);
    mc_axis_command_rate(&axis, 10.f);
    (void)mc_axis_step(&axis, 0.01f);
    mc_axis_off(&axis);
    EXPECT_FLOAT_EQ(mc_axis_step(&axis, 0.01f), 0.f);
}
