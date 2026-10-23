#include "mc/platform.h"

void mc_encoder_track_reset(mc_enc_track_t *track, int32_t max_delta)
{
    track->last = 0u;
    track->extended = 0;
    track->have_last = false;
    track->fault = false;
    track->max_delta = max_delta;
}

void mc_encoder_timer_init(TIM_TypeDef *tim)
{
    /* Encoder mode 3 counts on both TI1 and TI2 edges (RM0090). */
    tim->SMCR = TIM_SMCR_SMS_ENCODER3;
    tim->CCMR1 = TIM_CCMR1_CC1S_IN | TIM_CCMR1_CC2S_IN;
    tim->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E;
    tim->ARR = 0xFFFFu;
    tim->CNT = 0u;
    tim->CR1 = TIM_CR1_CEN;
}

int32_t mc_encoder_sample(TIM_TypeDef *tim, mc_enc_track_t *track)
{
    uint16_t now = (uint16_t)(tim->CNT & 0xFFFFu);
    int32_t delta;
    if (!track->have_last) {
        track->last = now;
        track->have_last = true;
        track->extended = 0;
        return 0;
    }
    delta = (int16_t)(uint16_t)(now - track->last);
    track->last = now;
    if ((track->max_delta > 0) && ((delta > track->max_delta) || (delta < -track->max_delta))) {
        track->fault = true; /* impossible step: noise or a broken encoder */
    }
    track->extended += delta;
    return track->extended;
}
