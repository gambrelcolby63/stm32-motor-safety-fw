#include "mc/analog.h"
#include "mc/config.h"

#include <gtest/gtest.h>

TEST(Analog, MidscaleCurrentIsZero)
{
    EXPECT_NEAR(mc_amps_from_raw(2048), 0.f, 1e-3);
}

TEST(Analog, OneAmpIsTheDocumentedSensitivity)
{
    /* 0.132 V at 3.3 V full scale, 12-bit, above midscale. */
    float counts = (0.132f / MC_ADC_VREF) * 4096.f;
    auto raw = static_cast<uint16_t>(2048.f + counts);
    EXPECT_NEAR(mc_amps_from_raw(raw), 1.f, 0.02);
}

TEST(Analog, TemperatureIsTenMillivoltsPerDegree)
{
    auto raw = static_cast<uint16_t>((0.25f / MC_ADC_VREF) * 4096.f);
    EXPECT_NEAR(mc_temp_c_from_raw(raw), 25.f, 0.05);
}
