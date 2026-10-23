#include "mc/platform.h"

void mc_pwm_tim1_init(uint32_t pwm_hz, uint32_t timer_hz)
{
    uint32_t arr = (timer_hz / pwm_hz) - 1u;
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    (void)RCC->APB2ENR;
    TIM1->PSC = 0u;
    TIM1->ARR = arr;
    TIM1->CCMR1 = TIM_CCMR1_OC1M_PWM1 | TIM_CCMR1_OC1PE | TIM_CCMR1_OC2M_PWM1 | TIM_CCMR1_OC2PE;
    TIM1->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E;
    TIM1->CCR1 = 0u;
    TIM1->CCR2 = 0u;
    TIM1->BDTR = TIM_BDTR_MOE; /* advanced timer outputs stay off until MOE is set */
    TIM1->CR1 = TIM_CR1_ARPE | TIM_CR1_CEN;
    TIM1->EGR = TIM_EGR_UG;
}

void mc_pwm_set(unsigned channel, float duty_percent)
{
    uint32_t arr = TIM1->ARR;
    uint32_t ccr;
    if (duty_percent < 0.f) {
        duty_percent = 0.f;
    } else if (duty_percent > 100.f) {
        duty_percent = 100.f;
    }
    ccr = (uint32_t)((duty_percent * (float)(arr + 1u)) / 100.f);
    if (ccr > arr) {
        ccr = arr;
    }
    if (channel == 1u) {
        TIM1->CCR1 = ccr;
    } else if (channel == 2u) {
        TIM1->CCR2 = ccr;
    }
}
