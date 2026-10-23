#include "mc/app.h"
#include "mc/config.h"
#include "mc/host_hw.h"
#include "mc/platform.h"

#include <gtest/gtest.h>

#include <cstring>
#include <string>

extern "C" void SysTick_Handler(void);

namespace {

void ticks(int count)
{
    for (int i = 0; i < count; ++i) {
        SysTick_Handler();
        mc_runtime_poll();
    }
}

void send(const char *text)
{
    std::string line = std::string(text) + "\n";
    for (char ch : line) {
        USART2->DR = static_cast<uint8_t>(ch);
        USART2->SR |= USART_SR_RXNE;
        SysTick_Handler();
        mc_runtime_poll();
    }
}

const char *log_text()
{
    return mc_uart_tx_log();
}

void boot()
{
    mc_host_hw_reset();
    mc_uart_tx_log_clear();
    mc_app_init();
    ticks(2);
}

void home_both_axes()
{
    send("home");
    EXPECT_NE(std::strstr(log_text(), "phase=tilt-seek"), nullptr);
    GPIOB->IDR |= (1u << 9);
    ticks(2);
    EXPECT_NE(std::strstr(log_text(), "phase=tilt-leave"), nullptr);
    GPIOB->IDR &= ~(1u << 9);
    ticks(3);
    EXPECT_NE(std::strstr(log_text(), "phase=pan-seek"), nullptr);
    GPIOB->IDR |= (1u << 1);
    ticks(2);
    EXPECT_NE(std::strstr(log_text(), "phase=pan-leave"), nullptr);
    GPIOB->IDR &= ~(1u << 1);
    ticks(4);
    EXPECT_EQ(mc_app_state(), MC_STATE_SAFE);
    EXPECT_TRUE(mc_app_homed());
}

}  // namespace

TEST(App, BootsToSafeAndRefusesEnable)
{
    boot();
    EXPECT_NE(std::strstr(log_text(), "BOOT pan-tilt-gimbal"), nullptr);
    EXPECT_EQ(mc_app_state(), MC_STATE_SAFE);
    EXPECT_EQ(mc_app_actuation(), MC_ACT_OFF);
    EXPECT_FALSE(mc_app_homed());
    mc_uart_tx_log_clear();
    send("enable");
    EXPECT_NE(std::strstr(log_text(), "ERR not-homed"), nullptr);
    EXPECT_EQ(mc_app_state(), MC_STATE_SAFE);
}

TEST(App, HomesThenEnablesAndRejectsKeepout)
{
    boot();
    home_both_axes();
    mc_uart_tx_log_clear();
    send("enable");
    EXPECT_EQ(mc_app_state(), MC_STATE_ENABLED);
    EXPECT_EQ(mc_app_actuation(), MC_ACT_HOLD);
    send("zone add 20 40 -10 10");
    mc_uart_tx_log_clear();
    send("pos 30 0");
    EXPECT_NE(std::strstr(log_text(), "ERR keep-out"), nullptr);
    EXPECT_EQ(mc_app_state(), MC_STATE_ENABLED);
    send("pos 5 5");
    ticks(2);
    EXPECT_EQ(mc_app_state(), MC_STATE_MOVING);
}

TEST(App, EstopLatchesUntilResetAndRequiresRehome)
{
    boot();
    home_both_axes();
    send("enable");
    EXPECT_TRUE(mc_app_homed());
    GPIOC->IDR |= 1u;
    ticks(2);
    EXPECT_EQ(mc_app_state(), MC_STATE_ESTOP);
    EXPECT_EQ(mc_app_actuation(), MC_ACT_OFF);
    EXPECT_FALSE(mc_app_homed());
    EXPECT_EQ(TIM1->CCR1, 0u);
    EXPECT_EQ(GPIOC->ODR & (1u << 2), 0u);

    mc_uart_tx_log_clear();
    send("reset");
    ticks(2);
    EXPECT_EQ(mc_app_state(), MC_STATE_ESTOP);

    GPIOC->IDR &= ~1u;
    ticks(1);
    EXPECT_EQ(mc_app_state(), MC_STATE_ESTOP);
    send("reset");
    ticks(2);
    EXPECT_EQ(mc_app_state(), MC_STATE_SAFE);
    EXPECT_FALSE(mc_app_homed());
    mc_uart_tx_log_clear();
    send("enable");
    EXPECT_NE(std::strstr(log_text(), "ERR not-homed"), nullptr);
}

TEST(App, DriverFaultAndAdcOvercurrentLatch)
{
    uint16_t hot_current = 3600u;
    boot();
    GPIOD->IDR |= 1u;
    ticks(2);
    EXPECT_EQ(mc_app_state(), MC_STATE_FAULT);
    EXPECT_EQ(mc_app_fault(), MC_FAULT_OVERCURRENT);
    EXPECT_EQ(mc_app_actuation(), MC_ACT_OFF);

    GPIOD->IDR &= ~1u;
    send("reset");
    ticks(2);
    EXPECT_EQ(mc_app_state(), MC_STATE_SAFE);

    mc_adc_host_set_channel(0u, hot_current);
    ticks(2);
    EXPECT_EQ(mc_app_fault(), MC_FAULT_OVERCURRENT);
}

TEST(App, OverTemperatureUsesControlledBraking)
{
    auto raw = static_cast<uint16_t>((0.90f / 3.3f) * 4096.f);
    boot();
    home_both_axes();
    send("enable");
    mc_adc_host_set_channel(4u, raw);
    ticks(2);
    EXPECT_EQ(mc_app_state(), MC_STATE_FAULT);
    EXPECT_EQ(mc_app_fault(), MC_FAULT_OVERTEMP);
    EXPECT_EQ(mc_app_actuation(), MC_ACT_BRAKE);
    ticks(static_cast<int>(MC_BRAKE_TIMEOUT_MS) + 5);
    EXPECT_EQ(mc_app_state(), MC_STATE_FAULT);
    EXPECT_EQ(mc_app_actuation(), MC_ACT_OFF);
}

TEST(App, HeartbeatTimeoutDeenergizes)
{
    boot();
    home_both_axes();
    send("enable");
    EXPECT_EQ(mc_app_state(), MC_STATE_ENABLED);
    ticks(static_cast<int>(MC_HEARTBEAT_MS) + 5);
    EXPECT_EQ(mc_app_state(), MC_STATE_SAFE);
    EXPECT_EQ(mc_app_actuation(), MC_ACT_OFF);
    EXPECT_EQ(mc_app_fault(), MC_FAULT_NONE);
}

TEST(App, WatchdogResetCauseLatchesUntilReset)
{
    mc_host_hw_reset();
    RCC->CSR |= RCC_CSR_IWDGRSTF;
    mc_app_init();
    ticks(2);
    EXPECT_EQ(mc_app_state(), MC_STATE_FAULT);
    EXPECT_EQ(mc_app_fault(), MC_FAULT_WATCHDOG);
    send("reset");
    ticks(2);
    EXPECT_EQ(mc_app_state(), MC_STATE_SAFE);
    EXPECT_EQ(mc_app_actuation(), MC_ACT_OFF);
}

TEST(App, RuntimeLoopKicksTheWatchdog)
{
    boot();
    EXPECT_EQ(IWDG->KR, IWDG_KR_KICK);
}

TEST(App, PositionOutsideSoftLimitsIsRejected)
{
    boot();
    home_both_axes();
    send("enable");
    mc_uart_tx_log_clear();
    send("pos 0 120");
    EXPECT_NE(std::strstr(log_text(), "ERR limit"), nullptr);
    EXPECT_EQ(mc_app_state(), MC_STATE_ENABLED);
}
