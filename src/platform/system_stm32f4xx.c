#include "stm32f407xx.h"

void SystemInit(void)
{
    uint32_t spins;
    /* Cortex-M4F: enable CP10/CP11 before any floating-point instruction. */
    SCB->CPACR |= (0xFUL << 20);
    __DSB();
    __ISB();
    /* Renode does not alias 0x00000000 to flash, so point VTOR at the image. */
    SCB->VTOR = 0x08000000UL;

    RCC->CR |= RCC_CR_HSION;
    for (spins = 0u; spins < 100000u; spins++) {
        if ((RCC->CR & RCC_CR_HSIRDY) != 0u) {
            break;
        }
    }
    RCC->CFGR &= ~3u; /* SYSCLK = HSI, 16 MHz. No PLL wait that can hang the sim. */
    FLASH->ACR = 0u;  /* 0 wait states is legal at 16 MHz. */
}
