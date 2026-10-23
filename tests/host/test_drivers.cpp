#include "mc/host_hw.h"
#include "mc/platform.h"
#include "mc/trace.h"

#include <gtest/gtest.h>

extern "C" void SysTick_Handler(void);

class Drivers : public ::testing::Test {
protected:
    void SetUp() override { mc_host_hw_reset(); }
};

TEST_F(Drivers, GpioModePullAlternateAndOutput)
{
    mc_gpio_mode(GPIOA, 8u, MC_GPIO_AF);
    EXPECT_EQ((GPIOA->MODER >> 16) & 3u, static_cast<uint32_t>(MC_GPIO_AF));
    mc_gpio_pull(GPIOC, 0u, MC_GPIO_PULL_UP);
    EXPECT_EQ(GPIOC->PUPDR & 3u, static_cast<uint32_t>(MC_GPIO_PULL_UP));
    mc_gpio_af(GPIOA, 9u, 1u);
    EXPECT_EQ((GPIOA->AFR[1] >> 4) & 0xFu, 1u);

    mc_gpio_write(GPIOD, 12u, true);
    EXPECT_EQ(GPIOD->BSRR, 1u << 12);
    EXPECT_NE(GPIOD->ODR & (1u << 12), 0u);
    mc_gpio_write(GPIOD, 12u, false);
    EXPECT_EQ(GPIOD->BSRR, 1u << (12u + 16u));
    EXPECT_EQ(GPIOD->ODR & (1u << 12), 0u);

    GPIOB->IDR = 1u << 10;
    EXPECT_TRUE(mc_gpio_read(GPIOB, 10u));
    EXPECT_FALSE(mc_gpio_read(GPIOB, 11u));
}

TEST_F(Drivers, PwmDutyAndMainOutputEnable)
{
    mc_pwm_tim1_init(20000u, 16000000u);
    EXPECT_EQ(TIM1->ARR, 799u);
    EXPECT_NE(TIM1->BDTR & TIM_BDTR_MOE, 0u);
    EXPECT_NE(TIM1->CR1 & TIM_CR1_CEN, 0u);
    EXPECT_NE(TIM1->CCER & TIM_CCER_CC1E, 0u);
    EXPECT_NE(RCC->APB2ENR & RCC_APB2ENR_TIM1EN, 0u);

    mc_pwm_set(1u, 0.f);
    EXPECT_EQ(TIM1->CCR1, 0u);
    mc_pwm_set(1u, 50.f);
    EXPECT_EQ(TIM1->CCR1, 400u);
    mc_pwm_set(2u, 100.f);
    EXPECT_EQ(TIM1->CCR2, 799u);
    mc_pwm_set(1u, -10.f);
    EXPECT_EQ(TIM1->CCR1, 0u);
}

TEST_F(Drivers, EncoderExtendsThroughWrapAndFlagsGlitches)
{
    mc_enc_track_t track;
    mc_encoder_timer_init(TIM3);
    EXPECT_EQ(TIM3->SMCR & 7u, 3u);
    EXPECT_NE(TIM3->CR1 & TIM_CR1_CEN, 0u);

    mc_encoder_track_reset(&track, 50);
    TIM3->CNT = 10;
    EXPECT_EQ(mc_encoder_sample(TIM3, &track), 0);
    TIM3->CNT = 25;
    EXPECT_EQ(mc_encoder_sample(TIM3, &track), 15);
    TIM3->CNT = 2; /* wrapped backwards through 0: 2 - 25 = -23 in 16-bit */
    EXPECT_EQ(mc_encoder_sample(TIM3, &track), 15 - 23);
    TIM3->CNT = static_cast<uint16_t>(2 + 200);
    EXPECT_EQ(mc_encoder_sample(TIM3, &track), 15 - 23 + 200);
    EXPECT_TRUE(track.fault);
}

TEST_F(Drivers, AdcSelectsTheChannelAndReturnsTheSample)
{
    uint16_t raw = 0u;
    EXPECT_TRUE(mc_adc_init());
    EXPECT_NE(ADC1->CR2 & ADC_CR2_ADON, 0u);
    mc_adc_host_set_channel(0u, 2500u);
    EXPECT_TRUE(mc_adc_read(0u, &raw));
    EXPECT_EQ(raw, 2500u);
    EXPECT_EQ(ADC1->SQR3 & 0x1Fu, 0u);
    EXPECT_FALSE(mc_adc_read(4u, &raw));
    mc_adc_host_set_channel(4u, 1117u);
    EXPECT_TRUE(mc_adc_read(4u, &raw));
    EXPECT_EQ(raw, 1117u);
    EXPECT_EQ(ADC1->SQR3 & 0x1Fu, 4u);
}

TEST_F(Drivers, UartBaudControlAndBytes)
{
    uint8_t byte = 0u;
    mc_uart_init(115200u, 16000000u);
    EXPECT_EQ(USART2->BRR, 139u);
    EXPECT_EQ(USART2->CR1 & (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE),
              USART_CR1_UE | USART_CR1_TE | USART_CR1_RE);
    mc_uart_write("Hi\n");
    EXPECT_STREQ(mc_uart_tx_log(), "Hi\n");
    EXPECT_EQ(USART2->DR, static_cast<uint32_t>('\n'));

    USART2->DR = static_cast<uint32_t>('A');
    USART2->SR |= USART_SR_RXNE;
    EXPECT_EQ(mc_uart_read(&byte), 1);
    EXPECT_EQ(byte, static_cast<uint8_t>('A'));
    EXPECT_EQ(mc_uart_read(&byte), 0);
}

TEST_F(Drivers, WatchdogStartAndKick)
{
    RCC->CSR |= RCC_CSR_IWDGRSTF;
    EXPECT_TRUE(mc_reset_was_watchdog());
    mc_reset_flags_clear();
    EXPECT_NE(RCC->CSR & RCC_CSR_RMVF, 0u);

    mc_iwdg_start();
    EXPECT_EQ(IWDG->PR, 6u);
    EXPECT_EQ(IWDG->RLR, 0x0FFFu);
    EXPECT_EQ(IWDG->KR, IWDG_KR_START);
    mc_iwdg_kick();
    EXPECT_EQ(IWDG->KR, IWDG_KR_KICK);
    mc_trace_hit("SR-WDG-2");
}

TEST_F(Drivers, SysTickAdvancesMillis)
{
    mc_systick_init(16000000u);
    EXPECT_EQ(mc_millis(), 0u);
    SysTick_Handler();
    SysTick_Handler();
    EXPECT_EQ(mc_millis(), 2u);
}

TEST_F(Drivers, BoardSelfTestPassesOnTheMock)
{
    mc_board_init();
    EXPECT_TRUE(mc_board_self_test_ok());
    EXPECT_FALSE(mc_board_watchdog_boot());
    EXPECT_NE(RCC->AHB1ENR & RCC_AHB1ENR_GPIOCEN, 0u);
    mc_motor_output(0, -40.f, true, true);
    EXPECT_EQ(GPIOC->ODR & (1u << 1), 0u); /* negative direction */
    EXPECT_NE(GPIOC->ODR & (1u << 2), 0u); /* enabled */
    EXPECT_NE(GPIOC->ODR & (1u << 4), 0u); /* brake released */
    EXPECT_GT(TIM1->CCR1, 0u);
    mc_motor_output(0, 10.f, false, true);
    EXPECT_EQ(GPIOC->ODR & ((1u << 2) | (1u << 4)), 0u);
    EXPECT_EQ(TIM1->CCR1, 0u);
}
