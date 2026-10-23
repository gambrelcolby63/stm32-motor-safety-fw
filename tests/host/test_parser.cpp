#include "mc/command.h"

#include <gtest/gtest.h>

TEST(Parser, MotionCommands)
{
    mc_parsed_cmd_t cmd;
    EXPECT_EQ(mc_parse_line("pos -12.5 30", &cmd), 1);
    EXPECT_EQ(cmd.verb, MC_VERB_POS);
    EXPECT_NEAR(cmd.a, -12.5f, 1e-4);
    EXPECT_NEAR(cmd.b, 30.f, 1e-4);

    EXPECT_EQ(mc_parse_line("rate 15 -4", &cmd), 1);
    EXPECT_EQ(cmd.verb, MC_VERB_RATE);

    EXPECT_EQ(mc_parse_line("mode pan continuous", &cmd), 1);
    EXPECT_EQ(cmd.flag, 1);
    EXPECT_EQ(mc_parse_line("mode pan limited", &cmd), 1);
    EXPECT_EQ(cmd.flag, 0);
    EXPECT_EQ(mc_parse_line("mode tilt continuous", &cmd), 0);
}

TEST(Parser, GainsLimitsZonesAndIndex)
{
    mc_parsed_cmd_t cmd;
    EXPECT_EQ(mc_parse_line("limits tilt -40 80", &cmd), 1);
    EXPECT_EQ(cmd.verb, MC_VERB_LIMITS);
    EXPECT_EQ(cmd.axis, 1);
    EXPECT_EQ(mc_parse_line("limits pan 10 5", &cmd), 0);

    EXPECT_EQ(mc_parse_line("zone add -10 10 20 40", &cmd), 1);
    EXPECT_EQ(cmd.verb, MC_VERB_ZONE_ADD);
    EXPECT_EQ(mc_parse_line("zone clear", &cmd), 1);
    EXPECT_EQ(cmd.verb, MC_VERB_ZONE_CLEAR);

    EXPECT_EQ(mc_parse_line("kp rate pan 0.4", &cmd), 1);
    EXPECT_EQ(cmd.verb, MC_VERB_KP);
    EXPECT_EQ(cmd.loop, 1);
    EXPECT_EQ(cmd.axis, 0);
    EXPECT_EQ(mc_parse_line("ki pos tilt 0.2", &cmd), 1);
    EXPECT_EQ(cmd.verb, MC_VERB_KI);

    EXPECT_EQ(mc_parse_line("index on", &cmd), 1);
    EXPECT_EQ(cmd.flag, 1);
    EXPECT_EQ(mc_parse_line("index off", &cmd), 1);
    EXPECT_EQ(cmd.flag, 0);
}

TEST(Parser, LifecycleWordsAndRejectsGarbage)
{
    mc_parsed_cmd_t cmd;
    EXPECT_EQ(mc_parse_line("help", &cmd), 1);
    EXPECT_EQ(cmd.verb, MC_VERB_HELP);
    EXPECT_EQ(mc_parse_line("status", &cmd), 1);
    EXPECT_EQ(mc_parse_line("home", &cmd), 1);
    EXPECT_EQ(mc_parse_line("enable", &cmd), 1);
    EXPECT_EQ(mc_parse_line("disable", &cmd), 1);
    EXPECT_EQ(mc_parse_line("stop", &cmd), 1);
    EXPECT_EQ(mc_parse_line("reset", &cmd), 1);
    EXPECT_EQ(mc_parse_line("hb", &cmd), 1);
    EXPECT_EQ(mc_parse_line("pos 1", &cmd), 0);
    EXPECT_EQ(mc_parse_line("nope", &cmd), 0);
    EXPECT_EQ(mc_parse_line("", &cmd), 0);
    EXPECT_EQ(mc_parse_line("enable extra", &cmd), 0);
}
