#include "mc/platform.h"

#include "mc/config.h"

static bool s_self_test_ok;
static bool s_watchdog_boot;

static void configure_pins(void)
{
    /* Active-high inputs. External hardware pulls the net down while healthy
     * or away from the switch; the sensed condition drives the pin high.
     * E-stop uses an external pull-up with the safety loop holding the pin
     * low, so an open wire reads as asserted. The on-chip pull-down is only
     * a default for the limit, home, index, and driver-fault nets. */
    mc_gpio_mode(GPIOC, 0u, MC_GPIO_INPUT);
    mc_gpio_pull(GPIOC, 0u, MC_GPIO_PULL_UP);

    mc_gpio_mode(GPIOB, 0u, MC_GPIO_INPUT); /* pan index */
    mc_gpio_mode(GPIOB, 1u, MC_GPIO_INPUT); /* pan home */
    mc_gpio_mode(GPIOB, 2u, MC_GPIO_INPUT); /* pan limit min */
    mc_gpio_mode(GPIOB, 3u, MC_GPIO_INPUT); /* pan limit max */
    mc_gpio_mode(GPIOB, 8u, MC_GPIO_INPUT); /* tilt index */
    mc_gpio_mode(GPIOB, 9u, MC_GPIO_INPUT); /* tilt home */
    mc_gpio_mode(GPIOB, 10u, MC_GPIO_INPUT); /* tilt limit min */
    mc_gpio_mode(GPIOB, 11u, MC_GPIO_INPUT); /* tilt limit max */
    mc_gpio_pull(GPIOB, 0u, MC_GPIO_PULL_DOWN);
    mc_gpio_pull(GPIOB, 1u, MC_GPIO_PULL_DOWN);
    mc_gpio_pull(GPIOB, 2u, MC_GPIO_PULL_DOWN);
    mc_gpio_pull(GPIOB, 3u, MC_GPIO_PULL_DOWN);
    mc_gpio_pull(GPIOB, 8u, MC_GPIO_PULL_DOWN);
    mc_gpio_pull(GPIOB, 9u, MC_GPIO_PULL_DOWN);
    mc_gpio_pull(GPIOB, 10u, MC_GPIO_PULL_DOWN);
    mc_gpio_pull(GPIOB, 11u, MC_GPIO_PULL_DOWN);

    mc_gpio_mode(GPIOD, 0u, MC_GPIO_INPUT); /* pan gate-driver fault */
    mc_gpio_mode(GPIOD, 1u, MC_GPIO_INPUT); /* tilt gate-driver fault */
    mc_gpio_pull(GPIOD, 0u, MC_GPIO_PULL_DOWN);
    mc_gpio_pull(GPIOD, 1u, MC_GPIO_PULL_DOWN);

    mc_gpio_mode(GPIOC, 1u, MC_GPIO_OUTPUT); /* pan direction */
    mc_gpio_mode(GPIOC, 2u, MC_GPIO_OUTPUT); /* pan enable */
    mc_gpio_mode(GPIOC, 4u, MC_GPIO_OUTPUT); /* pan brake release */
    mc_gpio_mode(GPIOC, 5u, MC_GPIO_OUTPUT); /* tilt direction */
    mc_gpio_mode(GPIOC, 6u, MC_GPIO_OUTPUT); /* tilt enable */
    mc_gpio_mode(GPIOC, 7u, MC_GPIO_OUTPUT); /* tilt brake release */

    mc_gpio_mode(GPIOD, 12u, MC_GPIO_OUTPUT);
    mc_gpio_mode(GPIOD, 13u, MC_GPIO_OUTPUT);
    mc_gpio_mode(GPIOD, 14u, MC_GPIO_OUTPUT);
    mc_gpio_mode(GPIOD, 15u, MC_GPIO_OUTPUT);

    mc_gpio_mode(GPIOA, 8u, MC_GPIO_AF);
    mc_gpio_af(GPIOA, 8u, 1u); /* TIM1_CH1 pan PWM */
    mc_gpio_mode(GPIOA, 9u, MC_GPIO_AF);
    mc_gpio_af(GPIOA, 9u, 1u); /* TIM1_CH2 tilt PWM */
    mc_gpio_mode(GPIOA, 6u, MC_GPIO_AF);
    mc_gpio_af(GPIOA, 6u, 2u); /* TIM3_CH1 pan A */
    mc_gpio_mode(GPIOA, 7u, MC_GPIO_AF);
    mc_gpio_af(GPIOA, 7u, 2u); /* TIM3_CH2 pan B */
    mc_gpio_mode(GPIOB, 6u, MC_GPIO_AF);
    mc_gpio_af(GPIOB, 6u, 2u); /* TIM4_CH1 tilt A */
    mc_gpio_mode(GPIOB, 7u, MC_GPIO_AF);
    mc_gpio_af(GPIOB, 7u, 2u); /* TIM4_CH2 tilt B */

    mc_gpio_mode(GPIOA, 2u, MC_GPIO_AF);
    mc_gpio_af(GPIOA, 2u, 7u); /* USART2_TX */
    mc_gpio_mode(GPIOA, 3u, MC_GPIO_AF);
    mc_gpio_af(GPIOA, 3u, 7u); /* USART2_RX */

    mc_gpio_mode(GPIOA, 0u, MC_GPIO_ANALOG); /* pan current */
    mc_gpio_mode(GPIOA, 1u, MC_GPIO_ANALOG); /* tilt current */
    mc_gpio_mode(GPIOA, 4u, MC_GPIO_ANALOG); /* temperature */
}

static bool run_self_test(void)
{
    mc_gpio_write(GPIOD, 12u, true);
    if ((GPIOD->ODR & (1u << 12)) == 0u) {
        return false;
    }
    mc_gpio_write(GPIOD, 12u, false);
    if ((GPIOD->ODR & (1u << 12)) != 0u) {
        return false;
    }
    if (TIM1->ARR == 0u) {
        return false;
    }
    return true;
}

void mc_board_init(void)
{
    s_watchdog_boot = mc_reset_was_watchdog();
    mc_reset_flags_clear();

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIODEN;
    (void)RCC->AHB1ENR;
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM4EN;
    (void)RCC->APB1ENR;

    configure_pins();
    mc_uart_init(MC_UART_BAUD, MC_SYSCLK_HZ);
    mc_pwm_tim1_init(MC_PWM_HZ, MC_SYSCLK_HZ);
    mc_encoder_timer_init(TIM3);
    mc_encoder_timer_init(TIM4);
    (void)mc_adc_init();
    mc_systick_init(MC_SYSCLK_HZ);
    s_self_test_ok = run_self_test();
    mc_iwdg_start();
}

bool mc_board_self_test_ok(void)
{
    return s_self_test_ok;
}

bool mc_board_watchdog_boot(void)
{
    return s_watchdog_boot;
}

void mc_motor_output(int axis, float duty_signed, bool enable, bool brake_release)
{
    unsigned dir_pin;
    unsigned en_pin;
    unsigned brake_pin;
    unsigned channel;
    float magnitude;

    if (!enable) {
        duty_signed = 0.f;
        brake_release = false;
    }
    if (axis == 0) {
        dir_pin = 1u;
        en_pin = 2u;
        brake_pin = 4u;
        channel = 1u;
    } else {
        dir_pin = 5u;
        en_pin = 6u;
        brake_pin = 7u;
        channel = 2u;
    }
    magnitude = (duty_signed >= 0.f) ? duty_signed : -duty_signed;
    mc_gpio_write(GPIOC, dir_pin, duty_signed >= 0.f);
    mc_pwm_set(channel, magnitude);
    mc_gpio_write(GPIOC, en_pin, enable);
    /* High releases a spring-applied brake. Low is the safe, engaged state. */
    mc_gpio_write(GPIOC, brake_pin, brake_release);
}

void mc_leds_show(bool heartbeat, bool enabled, bool fault, bool moving)
{
    mc_gpio_write(GPIOD, 12u, heartbeat);
    mc_gpio_write(GPIOD, 13u, enabled);
    mc_gpio_write(GPIOD, 14u, fault);
    mc_gpio_write(GPIOD, 15u, moving);
}
