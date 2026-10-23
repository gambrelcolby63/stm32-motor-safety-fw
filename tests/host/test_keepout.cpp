#include "mc/keepout.h"
#include "mc/trace.h"

#include <gtest/gtest.h>

TEST(Keepout, ContainsAndMisses)
{
    mc_keepout_t zones;
    mc_keepout_clear(&zones);
    EXPECT_TRUE(mc_keepout_add(&zones, 10.f, 20.f, -5.f, 5.f));
    EXPECT_TRUE(mc_keepout_contains(&zones, 15.f, 0.f));
    EXPECT_FALSE(mc_keepout_contains(&zones, 0.f, 0.f));
    EXPECT_FALSE(mc_keepout_contains(&zones, 21.f, 0.f));
}

TEST(Keepout, SegmentIntersection)
{
    mc_keepout_t zones;
    mc_keepout_clear(&zones);
    ASSERT_TRUE(mc_keepout_add(&zones, 10.f, 20.f, 10.f, 20.f));
    EXPECT_TRUE(mc_keepout_segment_hits(&zones, 0.f, 0.f, 30.f, 30.f, false));
    EXPECT_FALSE(mc_keepout_segment_hits(&zones, 0.f, 0.f, 0.f, 5.f, false));
    EXPECT_TRUE(mc_keepout_segment_hits(&zones, 0.f, 15.f, 30.f, 15.f, false));
    mc_trace_hit("SR-KO-1");
}

TEST(Keepout, WrappedPanSplitsAtTheBranchCut)
{
    mc_keepout_t zones;
    mc_keepout_clear(&zones);
    /* Zone sits across the +180 / -180 cut, expressed in [-180, 180]. */
    ASSERT_TRUE(mc_keepout_add(&zones, 170.f, 180.f, -1.f, 1.f));
    EXPECT_TRUE(mc_keepout_segment_hits(&zones, 170.f, 0.f, -170.f, 0.f, true));
    EXPECT_FALSE(mc_keepout_segment_hits(&zones, 0.f, 0.f, 10.f, 0.f, true));
}

TEST(Keepout, TableIsBounded)
{
    mc_keepout_t zones;
    int i;
    mc_keepout_clear(&zones);
    for (i = 0; i < MC_ZONE_MAX; i++) {
        EXPECT_TRUE(mc_keepout_add(&zones, 0.f, 1.f, 0.f, 1.f));
    }
    EXPECT_FALSE(mc_keepout_add(&zones, 0.f, 1.f, 0.f, 1.f));
    EXPECT_FALSE(mc_keepout_add(&zones, 2.f, 1.f, 0.f, 1.f));
    mc_keepout_clear(&zones);
    EXPECT_FALSE(mc_keepout_contains(&zones, 0.5f, 0.5f));
}
