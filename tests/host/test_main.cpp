#include "mc/trace.h"

#include <gtest/gtest.h>

#include <cstdio>

extern "C" int mc_trace_missing(char *buf, size_t len);

int main(int argc, char **argv)
{
    int failed;
    char missing[512];
    ::testing::InitGoogleTest(&argc, argv);
    failed = RUN_ALL_TESTS();
    if (mc_trace_missing(missing, sizeof(missing)) != 0) {
        std::fprintf(stderr, "Safety requirements with no test hit: %s\n", missing);
        return 1;
    }
    return failed;
}
