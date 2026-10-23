#include "mc/platform.h"

#if defined(MC_HOST_BUILD)
#include "mc/host_hw.h"

static uint16_t s_host_sample[19];
static uint32_t s_host_valid;

void mc_adc_host_set_channel(uint8_t channel, uint16_t raw)
{
    if (channel < 19u) {
        s_host_sample[channel] = raw;
        s_host_valid |= (1u << channel);
    }
}

void mc_adc_host_set_invalid(void)
{
    s_host_valid = 0u;
}
#endif

bool mc_adc_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    (void)RCC->APB2ENR;
    /* 480-cycle sample time on IN0, IN1, and IN4. */
    ADC1->SMPR2 |= (7u << 0) | (7u << 3) | (7u << 12);
    ADC1->CR2 |= ADC_CR2_ADON;
    ADC1->SQR1 = 0u;
    return true;
}

static void adc_start(uint8_t channel)
{
    ADC1->SQR1 = 0u;
    ADC1->SQR3 = (uint32_t)channel & 0x1Fu;
    ADC1->SR &= ~ADC_SR_EOC;
    ADC1->CR2 |= ADC_CR2_SWSTART;
}

bool mc_adc_read(uint8_t channel, uint16_t *raw)
{
    uint32_t spins;
    adc_start(channel);
#if defined(MC_HOST_BUILD)
    /* The mock ADC has no conversion clock. A preloaded channel sample completes the poll. */
    if ((channel < 19u) && ((s_host_valid & (1u << channel)) != 0u)) {
        ADC1->DR = s_host_sample[channel];
        ADC1->SR |= ADC_SR_EOC;
    }
#endif
    /* One conversion is tens of microseconds at 16 MHz. A short poll is enough
     * when the converter sets EOC; a missing converter must not stall the loop. */
    for (spins = 0u; spins < 64u; spins++) {
        if ((ADC1->SR & ADC_SR_EOC) != 0u) {
            *raw = (uint16_t)(ADC1->DR & 0x0FFFu);
            return true;
        }
    }
    return false;
}

bool mc_adc_service(uint8_t *channel, uint16_t *raw)
{
    /* Pan current, tilt current, temperature. One conversion is in flight at a time
     * so the 1 ms loop never busy-waits on a converter that does not complete. */
    static const uint8_t k_channels[3] = {0u, 1u, 4u};
    static uint8_t index;
    static bool started;

#if defined(MC_HOST_BUILD)
    uint8_t n;
    (void)started;
    for (n = 0u; n < 3u; n++) {
        uint8_t ch = k_channels[(index + n) % 3u];
        if ((ch < 19u) && ((s_host_valid & (1u << ch)) != 0u)) {
            *channel = ch;
            *raw = s_host_sample[ch];
            index = (uint8_t)((index + n + 1u) % 3u);
            return true;
        }
    }
    return false;
#else
    if (started && ((ADC1->SR & ADC_SR_EOC) != 0u)) {
        *channel = k_channels[index];
        *raw = (uint16_t)(ADC1->DR & 0x0FFFu);
        index = (uint8_t)((index + 1u) % 3u);
        adc_start(k_channels[index]);
        return true;
    }
    if (started) {
        index = (uint8_t)((index + 1u) % 3u);
    }
    adc_start(k_channels[index]);
    started = true;
    return false;
#endif
}
