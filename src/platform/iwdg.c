#include "mc/platform.h"

void mc_iwdg_start(void)
{
    /* Prescaler /256 and a full reload is about 32 s at the 32 kHz LSI (RM0090). */
    IWDG->KR = IWDG_KR_UNLOCK;
    IWDG->PR = 6u;
    IWDG->RLR = 0x0FFFu;
    IWDG->KR = IWDG_KR_START;
}

void mc_iwdg_kick(void)
{
    IWDG->KR = IWDG_KR_KICK;
}

bool mc_reset_was_watchdog(void)
{
    return (RCC->CSR & RCC_CSR_IWDGRSTF) != 0u;
}

void mc_reset_flags_clear(void)
{
    RCC->CSR |= RCC_CSR_RMVF;
}
