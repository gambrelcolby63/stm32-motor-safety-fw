#include "mc/analog.h"

#include "mc/config.h"

float mc_amps_from_raw(uint16_t raw)
{
    float volts = ((float)raw * MC_ADC_VREF) / 4096.f;
    float mid = MC_ADC_VREF * 0.5f;
    return (volts - mid) / MC_I_SENSE_V_PER_A;
}

float mc_temp_c_from_raw(uint16_t raw)
{
    float volts = ((float)raw * MC_ADC_VREF) / 4096.f;
    return volts / MC_TEMP_V_PER_C;
}
