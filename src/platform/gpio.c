#include "mc/platform.h"

void mc_gpio_mode(GPIO_TypeDef *port, unsigned pin, mc_gpio_mode_t mode)
{
    uint32_t shift = pin * 2u;
    uint32_t mask = 3u << shift;
    port->MODER = (port->MODER & ~mask) | ((uint32_t)mode << shift);
}

void mc_gpio_pull(GPIO_TypeDef *port, unsigned pin, mc_gpio_pull_t pull)
{
    uint32_t shift = pin * 2u;
    uint32_t mask = 3u << shift;
    port->PUPDR = (port->PUPDR & ~mask) | ((uint32_t)pull << shift);
}

void mc_gpio_af(GPIO_TypeDef *port, unsigned pin, uint32_t af)
{
    unsigned index = pin >> 3;
    uint32_t shift = (pin & 7u) * 4u;
    port->AFR[index] = (port->AFR[index] & ~(0xFu << shift)) | ((af & 0xFu) << shift);
}

void mc_gpio_write(GPIO_TypeDef *port, unsigned pin, bool high)
{
    /* BSRR is the atomic set/reset register (RM0090). Set wins if both halves are written. */
    if (high) {
        port->BSRR = 1u << pin;
    } else {
        port->BSRR = 1u << (pin + 16u);
    }
#if defined(MC_HOST_BUILD)
    /* Host register mocks are plain memory and do not implement BSRR side effects. */
    if (high) {
        port->ODR |= (1u << pin);
    } else {
        port->ODR &= ~(1u << pin);
    }
#endif
}

bool mc_gpio_read(const GPIO_TypeDef *port, unsigned pin)
{
    return (port->IDR & (1u << pin)) != 0u;
}
