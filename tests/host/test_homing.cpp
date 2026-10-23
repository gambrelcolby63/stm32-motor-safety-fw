#include "mc/homing.h"
#include "mc/trace.h"

#include <gtest/gtest.h>

static mc_home_sense_t sense_with(bool tilt_home, bool tilt_index, bool pan_home, bool pan_index)
{
    mc_home_sense_t sense{};
    sense.tilt_home = tilt_home;
    sense.tilt_index = tilt_index;
    sense.pan_home = pan_home;
    sense.pan_index = pan_index;
    sense.seek_dps = 30.f;
    sense.creep_dps = 8.f;
    sense.timeout_ticks = 20;
    return sense;
}

TEST(Homing, SwitchSequenceZerosTiltThenPan)
{
    mc_homing_t home;
    mc_home_out_t out;
    mc_home_sense_t sense;
    mc_homing_start(&home, false);

    sense = sense_with(false, false, false, false);
    mc_homing_step(&home, &sense, &out);
    EXPECT_EQ(out.phase, MC_HOME_SEEK);
    EXPECT_LT(out.tilt_rate_dps, 0.f);
    EXPECT_FLOAT_EQ(out.pan_rate_dps, 0.f);

    sense = sense_with(true, false, false, false);
    mc_homing_step(&home, &sense, &out);
    EXPECT_EQ(out.phase, MC_HOME_LEAVE);
    EXPECT_GT(out.tilt_rate_dps, 0.f);

    sense = sense_with(false, false, false, false);
    mc_homing_step(&home, &sense, &out);
    EXPECT_TRUE(out.zero_tilt);
    EXPECT_EQ(out.which, 2);
    EXPECT_EQ(out.phase, MC_HOME_SEEK);

    sense = sense_with(false, false, false, false);
    mc_homing_step(&home, &sense, &out);
    EXPECT_EQ(out.phase, MC_HOME_SEEK);
    EXPECT_LT(out.pan_rate_dps, 0.f);

    sense = sense_with(false, false, true, false);
    mc_homing_step(&home, &sense, &out);
    EXPECT_EQ(out.phase, MC_HOME_LEAVE);

    sense = sense_with(false, false, false, false);
    mc_homing_step(&home, &sense, &out);
    EXPECT_TRUE(out.zero_pan);
    EXPECT_TRUE(out.done);
    EXPECT_EQ(out.phase, MC_HOME_DONE);
    mc_trace_hit("SR-HOME-2");
}

TEST(Homing, IndexPulseIsTheZeroWhenEnabled)
{
    mc_homing_t home;
    mc_home_out_t out;
    mc_home_sense_t sense;
    mc_homing_start(&home, true);

    sense = sense_with(true, false, false, false);
    mc_homing_step(&home, &sense, &out);
    EXPECT_EQ(out.phase, MC_HOME_BACKOFF);

    sense = sense_with(false, false, false, false);
    mc_homing_step(&home, &sense, &out);
    EXPECT_EQ(out.phase, MC_HOME_SEEK);

    sense = sense_with(true, false, false, false);
    mc_homing_step(&home, &sense, &out);
    EXPECT_EQ(out.phase, MC_HOME_LEAVE);

    sense = sense_with(false, false, false, false);
    mc_homing_step(&home, &sense, &out);
    EXPECT_EQ(out.phase, MC_HOME_INDEX);
    EXPECT_FALSE(out.zero_tilt);

    sense = sense_with(false, true, false, false);
    mc_homing_step(&home, &sense, &out);
    EXPECT_TRUE(out.zero_tilt);
}

TEST(Homing, TimeoutIsAFailure)
{
    mc_homing_t home;
    mc_home_out_t out{};
    mc_home_sense_t sense = sense_with(false, false, false, false);
    int i;
    sense.timeout_ticks = 3;
    mc_homing_start(&home, false);
    for (i = 0; i < 4; i++) {
        mc_homing_step(&home, &sense, &out);
    }
    EXPECT_TRUE(out.failed);
    EXPECT_EQ(out.phase, MC_HOME_FAIL);
    mc_trace_hit("SR-HOME-3");
}
