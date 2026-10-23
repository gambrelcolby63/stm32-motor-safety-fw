#include "mc/trace.h"
#include "mc/trajectory.h"

#include <gtest/gtest.h>

#include <cmath>

TEST(Trajectory, SlewLimitsAcceleration)
{
    float v = 0.f;
    int i;
    for (i = 0; i < 5; i++) {
        v = mc_slew(v, 100.f, 10.f, 1.f);
    }
    EXPECT_NEAR(v, 50.f, 1e-3);
}

TEST(Trajectory, TrapezoidRespectsRateAndAcceleration)
{
    mc_trap_t trap;
    float prev = 0.f;
    float peak = 0.f;
    float peak_acc = 0.f;
    int i;
    mc_trap_init(&trap, 0.f, 40.f, 80.f);
    mc_trap_set_target(&trap, 90.f);
    for (i = 0; i < 8000; i++) {
        mc_trap_step(&trap, 0.001f);
        peak = std::fmax(peak, std::fabs(trap.vel));
        peak_acc = std::fmax(peak_acc, std::fabs(trap.vel - prev) / 0.001f);
        prev = trap.vel;
        if (trap.done) {
            break;
        }
    }
    EXPECT_TRUE(trap.done);
    EXPECT_NEAR(trap.pos, 90.f, 0.1);
    EXPECT_LE(peak, 40.05f);          /* SR-RATE-1 */
    EXPECT_LE(peak_acc, 80.5f);
    mc_trace_hit("SR-RATE-1");
}

TEST(Trajectory, TrapezoidReversesDirection)
{
    mc_trap_t trap;
    int i;
    mc_trap_init(&trap, 0.f, 30.f, 60.f);
    trap.vel = 20.f;
    mc_trap_set_target(&trap, -10.f);
    for (i = 0; i < 5000; i++) {
        mc_trap_step(&trap, 0.001f);
        EXPECT_LE(std::fabs(trap.vel), 30.05f);
        if (trap.done) {
            break;
        }
    }
    EXPECT_TRUE(trap.done);
    EXPECT_NEAR(trap.pos, -10.f, 0.1);
}

TEST(Trajectory, SoftLimitStopsBeforeTheTravelEnd)
{
    float capped = mc_limit_velocity(10.f, 50.f, -10.f, 10.f, 20.f);
    float inside = mc_limit_velocity(0.f, 100.f, -20.f, 8.f, 2.f);
    EXPECT_NEAR(capped, 0.f, 1e-3); /* already on the positive stop */
    EXPECT_NEAR(inside, std::sqrt(2.f * 2.f * 8.f), 1e-3);
    EXPECT_LT(inside, 100.f);
    mc_trace_hit("SR-LIM-2");
}

TEST(Trajectory, Wrap180)
{
    EXPECT_NEAR(mc_wrap180(190.f), -170.f, 1e-3);
    EXPECT_NEAR(mc_wrap180(-190.f), 170.f, 1e-3);
    EXPECT_NEAR(mc_wrap180(10.f), 10.f, 1e-3);
    /* Half-open range [-180, 180): +180 and -180 are the same heading. */
    EXPECT_NEAR(fabsf(mc_wrap180(540.f)), 180.f, 1e-3);
}
