#include "mc/pi.h"

#include <gtest/gtest.h>

TEST(Pi, ProportionalOnly)
{
    mc_pi_t pi;
    mc_pi_init(&pi, 2.f, 0.f, -100.f, 100.f);
    EXPECT_NEAR(mc_pi_step(&pi, 3.f, 0.01f), 6.f, 1e-4);
}

TEST(Pi, IntegratesError)
{
    mc_pi_t pi;
    float out;
    mc_pi_init(&pi, 0.f, 2.f, -100.f, 100.f);
    out = mc_pi_step(&pi, 1.f, 0.5f);
    EXPECT_NEAR(out, 1.f, 1e-4);
    EXPECT_NEAR(pi.integral, 1.f, 1e-4);
}

TEST(Pi, AntiWindupFreezesIntegralWhenSaturated)
{
    mc_pi_t pi;
    int i;
    mc_pi_init(&pi, 0.f, 10.f, -1.f, 1.f);
    for (i = 0; i < 20; i++) {
        EXPECT_NEAR(mc_pi_step(&pi, 1.f, 0.1f), 1.f, 1e-4);
    }
    EXPECT_LE(pi.integral, 1.f);
    /* Error reverses, so the integrator is allowed to unwind. */
    EXPECT_LT(mc_pi_step(&pi, -1.f, 0.1f), 1.f);
}

TEST(Pi, ResetClearsIntegral)
{
    mc_pi_t pi;
    mc_pi_init(&pi, 0.f, 1.f, -10.f, 10.f);
    (void)mc_pi_step(&pi, 1.f, 1.f);
    mc_pi_reset(&pi);
    EXPECT_FLOAT_EQ(pi.integral, 0.f);
}
