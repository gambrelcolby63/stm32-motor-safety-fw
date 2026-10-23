#include "mc/host_hw.h"
#include "stm32f407xx.h"

#include <string.h>

#if defined(MC_HOST_BUILD)

GPIO_TypeDef g_hw_gpioa;
GPIO_TypeDef g_hw_gpiob;
GPIO_TypeDef g_hw_gpioc;
GPIO_TypeDef g_hw_gpiod;
TIM_TypeDef g_hw_tim1;
TIM_TypeDef g_hw_tim3;
TIM_TypeDef g_hw_tim4;
ADC_TypeDef g_hw_adc1;
USART_TypeDef g_hw_usart2;
IWDG_TypeDef g_hw_iwdg;
RCC_TypeDef g_hw_rcc;
FLASH_TypeDef g_hw_flash;

void mc_host_hw_reset(void)
{
    memset(&g_hw_gpioa, 0, sizeof(g_hw_gpioa));
    memset(&g_hw_gpiob, 0, sizeof(g_hw_gpiob));
    memset(&g_hw_gpioc, 0, sizeof(g_hw_gpioc));
    memset(&g_hw_gpiod, 0, sizeof(g_hw_gpiod));
    memset(&g_hw_tim1, 0, sizeof(g_hw_tim1));
    memset(&g_hw_tim3, 0, sizeof(g_hw_tim3));
    memset(&g_hw_tim4, 0, sizeof(g_hw_tim4));
    memset(&g_hw_adc1, 0, sizeof(g_hw_adc1));
    memset(&g_hw_usart2, 0, sizeof(g_hw_usart2));
    memset(&g_hw_iwdg, 0, sizeof(g_hw_iwdg));
    memset(&g_hw_rcc, 0, sizeof(g_hw_rcc));
    memset(&g_hw_flash, 0, sizeof(g_hw_flash));
    mc_adc_host_set_invalid();
}

#endif
