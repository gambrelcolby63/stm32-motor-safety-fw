#include "mc/platform.h"

#if defined(MC_HOST_BUILD)
typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
} SysTick_Type;

static SysTick_Type s_systick;
#define SysTick (&s_systick)
#define SysTick_CTRL_CLKSOURCE_Msk (1u << 2)
#define SysTick_CTRL_TICKINT_Msk (1u << 1)
#define SysTick_CTRL_ENABLE_Msk (1u << 0)
#endif

static volatile uint32_t s_millis;

void mc_systick_init(uint32_t clock_hz)
{
    uint32_t reload = (clock_hz / 1000u) - 1u;
    SysTick->LOAD = reload & 0x00FFFFFFu;
    SysTick->VAL = 0u;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

uint32_t mc_millis(void)
{
    return s_millis;
}

void SysTick_Handler(void)
{
    s_millis++;
}
