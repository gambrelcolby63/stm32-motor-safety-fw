#ifndef MC_PLATFORM_H
#define MC_PLATFORM_H

#include "stm32f407xx.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MC_GPIO_INPUT = 0,
    MC_GPIO_OUTPUT = 1,
    MC_GPIO_AF = 2,
    MC_GPIO_ANALOG = 3
} mc_gpio_mode_t;

typedef enum {
    MC_GPIO_PULL_NONE = 0,
    MC_GPIO_PULL_UP = 1,
    MC_GPIO_PULL_DOWN = 2
} mc_gpio_pull_t;

typedef struct {
    uint16_t last;
    int32_t extended;
    bool have_last;
    bool fault;
    int32_t max_delta;
} mc_enc_track_t;

void mc_gpio_clocks_enable(void);
void mc_gpio_mode(GPIO_TypeDef *port, unsigned pin, mc_gpio_mode_t mode);
void mc_gpio_pull(GPIO_TypeDef *port, unsigned pin, mc_gpio_pull_t pull);
void mc_gpio_af(GPIO_TypeDef *port, unsigned pin, uint32_t af);
void mc_gpio_write(GPIO_TypeDef *port, unsigned pin, bool high);
bool mc_gpio_read(const GPIO_TypeDef *port, unsigned pin);

void mc_pwm_tim1_init(uint32_t pwm_hz, uint32_t timer_hz);
void mc_pwm_set(unsigned channel, float duty_percent);

void mc_encoder_timer_init(TIM_TypeDef *tim);
int32_t mc_encoder_sample(TIM_TypeDef *tim, mc_enc_track_t *track);
void mc_encoder_track_reset(mc_enc_track_t *track, int32_t max_delta);

bool mc_adc_init(void);
bool mc_adc_read(uint8_t channel, uint16_t *raw);
/* Round-robin sample of the current and temperature channels. False keeps the last value. */
bool mc_adc_service(uint8_t *channel, uint16_t *raw);

void mc_uart_init(uint32_t baud, uint32_t clock_hz);
void mc_uart_write(const char *text);
int mc_uart_read(uint8_t *byte_out);
const char *mc_uart_tx_log(void);
void mc_uart_tx_log_clear(void);

void mc_systick_init(uint32_t clock_hz);
uint32_t mc_millis(void);

void mc_iwdg_start(void);
void mc_iwdg_kick(void);
bool mc_reset_was_watchdog(void);
void mc_reset_flags_clear(void);

void mc_board_init(void);
bool mc_board_self_test_ok(void);
bool mc_board_watchdog_boot(void);
void mc_motor_output(int axis, float duty_signed, bool enable, bool brake_release);
void mc_leds_show(bool heartbeat, bool enabled, bool fault, bool moving);

#ifdef __cplusplus
}
#endif

#endif /* MC_PLATFORM_H */
