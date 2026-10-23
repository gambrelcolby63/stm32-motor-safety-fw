/*
 * CMSIS-style register map for the STM32F407 peripherals this firmware uses.
 * Layout matches RM0090. Offsets are checked with static assertions.
 * This is not the STM32 HAL. Drivers write these registers directly.
 * On the target, CMSIS-Core (core_cm4.h) provides SCB, NVIC, and SysTick.
 */
#ifndef STM32F407XX_H
#define STM32F407XX_H

#include <stdint.h>
#include <stddef.h>

#if !defined(MC_HOST_BUILD)
#if !defined(__FPU_PRESENT)
#define __FPU_PRESENT 1U
#endif
#if !defined(__MPU_PRESENT)
#define __MPU_PRESENT 1U
#endif
#if !defined(__NVIC_PRIO_BITS)
#define __NVIC_PRIO_BITS 4U
#endif
#if !defined(__Vendor_SysTickConfig)
#define __Vendor_SysTickConfig 0U
#endif

/* CMSIS requires IRQn_Type before core_cm4.h. Numbers match RM0090 Table 61. */
typedef enum {
    NonMaskableInt_IRQn   = -14,
    HardFault_IRQn         = -13,
    MemoryManagement_IRQn  = -12,
    BusFault_IRQn          = -11,
    UsageFault_IRQn        = -10,
    SVCall_IRQn            = -5,
    DebugMonitor_IRQn      = -4,
    PendSV_IRQn            = -2,
    SysTick_IRQn           = -1,
    WWDG_IRQn              = 0,
    PVD_IRQn               = 1,
    TAMP_STAMP_IRQn        = 2,
    RTC_WKUP_IRQn          = 3,
    FLASH_IRQn             = 4,
    RCC_IRQn               = 5,
    EXTI0_IRQn             = 6,
    EXTI1_IRQn             = 7,
    EXTI2_IRQn             = 8,
    EXTI3_IRQn             = 9,
    EXTI4_IRQn             = 10,
    DMA1_Stream0_IRQn      = 11,
    DMA1_Stream1_IRQn      = 12,
    DMA1_Stream2_IRQn      = 13,
    DMA1_Stream3_IRQn      = 14,
    DMA1_Stream4_IRQn      = 15,
    DMA1_Stream5_IRQn      = 16,
    DMA1_Stream6_IRQn      = 17,
    ADC_IRQn               = 18,
    CAN1_TX_IRQn           = 19,
    CAN1_RX0_IRQn          = 20,
    CAN1_RX1_IRQn          = 21,
    CAN1_SCE_IRQn          = 22,
    EXTI9_5_IRQn           = 23,
    TIM1_BRK_TIM9_IRQn     = 24,
    TIM1_UP_TIM10_IRQn     = 25,
    TIM1_TRG_COM_TIM11_IRQn = 26,
    TIM1_CC_IRQn           = 27,
    TIM2_IRQn              = 28,
    TIM3_IRQn              = 29,
    TIM4_IRQn              = 30,
    I2C1_EV_IRQn           = 31,
    I2C1_ER_IRQn           = 32,
    I2C2_EV_IRQn           = 33,
    I2C2_ER_IRQn           = 34,
    SPI1_IRQn              = 35,
    SPI2_IRQn              = 36,
    USART1_IRQn            = 37,
    USART2_IRQn            = 38,
    USART3_IRQn            = 39,
    EXTI15_10_IRQn         = 40,
    RTC_Alarm_IRQn         = 41,
    OTG_FS_WKUP_IRQn       = 42,
    TIM8_BRK_TIM12_IRQn    = 43,
    TIM8_UP_TIM13_IRQn     = 44,
    TIM8_TRG_COM_TIM14_IRQn = 45,
    TIM8_CC_IRQn           = 46,
    DMA1_Stream7_IRQn      = 47,
    FSMC_IRQn              = 48,
    SDIO_IRQn              = 49,
    TIM5_IRQn              = 50,
    SPI3_IRQn              = 51,
    UART4_IRQn             = 52,
    UART5_IRQn             = 53,
    TIM6_DAC_IRQn          = 54,
    TIM7_IRQn              = 55,
    DMA2_Stream0_IRQn      = 56,
    DMA2_Stream1_IRQn      = 57,
    DMA2_Stream2_IRQn      = 58,
    DMA2_Stream3_IRQn      = 59,
    DMA2_Stream4_IRQn      = 60,
    ETH_IRQn               = 61,
    ETH_WKUP_IRQn          = 62,
    CAN2_TX_IRQn           = 63,
    CAN2_RX0_IRQn          = 64,
    CAN2_RX1_IRQn          = 65,
    CAN2_SCE_IRQn          = 66,
    OTG_FS_IRQn            = 67,
    DMA2_Stream5_IRQn      = 68,
    DMA2_Stream6_IRQn      = 69,
    DMA2_Stream7_IRQn      = 70,
    USART6_IRQn            = 71,
    I2C3_EV_IRQn           = 72,
    I2C3_ER_IRQn           = 73,
    OTG_HS_EP1_OUT_IRQn    = 74,
    OTG_HS_EP1_IN_IRQn     = 75,
    OTG_HS_WKUP_IRQn       = 76,
    OTG_HS_IRQn            = 77,
    DCMI_IRQn              = 78,
    CRYP_IRQn              = 79,
    HASH_RNG_IRQn          = 80,
    FPU_IRQn               = 81
} IRQn_Type;

#include "core_cm4.h"
#else
#define __IO volatile
#define __I volatile const
#define __O volatile
#define __IM volatile const
#define __OM volatile
#define __IOM volatile
#endif

#if defined(__cplusplus)
#define MC_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#else
#define MC_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#endif

/* -------------------------------------------------------------------------- */
/* Bases (RM0090 memory map)                                                  */
/* -------------------------------------------------------------------------- */

#define PERIPH_BASE        0x40000000UL
#define APB1PERIPH_BASE    PERIPH_BASE
#define APB2PERIPH_BASE    (PERIPH_BASE + 0x00010000UL)
#define AHB1PERIPH_BASE    (PERIPH_BASE + 0x00020000UL)

#define TIM3_BASE          (APB1PERIPH_BASE + 0x0400UL)
#define TIM4_BASE          (APB1PERIPH_BASE + 0x0800UL)
#define IWDG_BASE          (APB1PERIPH_BASE + 0x3000UL)
#define USART2_BASE        (APB1PERIPH_BASE + 0x4400UL)

#define TIM1_BASE          (APB2PERIPH_BASE + 0x0000UL)
#define ADC1_BASE          (APB2PERIPH_BASE + 0x2000UL)

#define GPIOA_BASE         (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOB_BASE         (AHB1PERIPH_BASE + 0x0400UL)
#define GPIOC_BASE         (AHB1PERIPH_BASE + 0x0800UL)
#define GPIOD_BASE         (AHB1PERIPH_BASE + 0x0C00UL)
#define RCC_BASE           (AHB1PERIPH_BASE + 0x3800UL)
#define FLASH_R_BASE       (AHB1PERIPH_BASE + 0x3C00UL)

/* -------------------------------------------------------------------------- */
/* GPIO                                                                       */
/* -------------------------------------------------------------------------- */

typedef struct {
    __IO uint32_t MODER;
    __IO uint32_t OTYPER;
    __IO uint32_t OSPEEDR;
    __IO uint32_t PUPDR;
    __IO uint32_t IDR;
    __IO uint32_t ODR;
    __IO uint32_t BSRR;
    __IO uint32_t LCKR;
    __IO uint32_t AFR[2];
} GPIO_TypeDef;

MC_STATIC_ASSERT(offsetof(GPIO_TypeDef, BSRR) == 0x18, "GPIO BSRR offset");
MC_STATIC_ASSERT(offsetof(GPIO_TypeDef, AFR) == 0x20, "GPIO AFR offset");

/* -------------------------------------------------------------------------- */
/* TIM (advanced and general-purpose share the offsets this firmware uses)   */
/* -------------------------------------------------------------------------- */

typedef struct {
    __IO uint32_t CR1;
    __IO uint32_t CR2;
    __IO uint32_t SMCR;
    __IO uint32_t DIER;
    __IO uint32_t SR;
    __IO uint32_t EGR;
    __IO uint32_t CCMR1;
    __IO uint32_t CCMR2;
    __IO uint32_t CCER;
    __IO uint32_t CNT;
    __IO uint32_t PSC;
    __IO uint32_t ARR;
    __IO uint32_t RCR;
    __IO uint32_t CCR1;
    __IO uint32_t CCR2;
    __IO uint32_t CCR3;
    __IO uint32_t CCR4;
    __IO uint32_t BDTR;
    __IO uint32_t DCR;
    __IO uint32_t DMAR;
} TIM_TypeDef;

MC_STATIC_ASSERT(offsetof(TIM_TypeDef, CCR1) == 0x34, "TIM CCR1 offset");
MC_STATIC_ASSERT(offsetof(TIM_TypeDef, BDTR) == 0x44, "TIM BDTR offset");

#define TIM_CR1_CEN            (1u << 0)
#define TIM_CR1_ARPE           (1u << 7)
#define TIM_SMCR_SMS_Msk       (7UL << 0)
#define TIM_SMCR_SMS_ENCODER3  (3UL << 0)
#define TIM_CCMR1_CC1S_IN      (1u << 0)
#define TIM_CCMR1_CC2S_IN      (1u << 8)
#define TIM_CCMR1_OC1PE        (1u << 3)
#define TIM_CCMR1_OC1M_PWM1    (6UL << 4)   /* 110: PWM mode 1 */
#define TIM_CCMR1_OC2PE        (1u << 11)
#define TIM_CCMR1_OC2M_PWM1    (6UL << 12)
#define TIM_CCER_CC1E          (1u << 0)
#define TIM_CCER_CC2E          (1u << 4)
#define TIM_EGR_UG             (1u << 0)
#define TIM_BDTR_MOE           (1u << 15)

/* -------------------------------------------------------------------------- */
/* ADC1                                                                       */
/* -------------------------------------------------------------------------- */

typedef struct {
    __IO uint32_t SR;
    __IO uint32_t CR1;
    __IO uint32_t CR2;
    __IO uint32_t SMPR1;
    __IO uint32_t SMPR2;
    __IO uint32_t JOFR1;
    __IO uint32_t JOFR2;
    __IO uint32_t JOFR3;
    __IO uint32_t JOFR4;
    __IO uint32_t HTR;
    __IO uint32_t LTR;
    __IO uint32_t SQR1;
    __IO uint32_t SQR2;
    __IO uint32_t SQR3;
    __IO uint32_t JSQR;
    __IO uint32_t JDR1;
    __IO uint32_t JDR2;
    __IO uint32_t JDR3;
    __IO uint32_t JDR4;
    __IO uint32_t DR;
} ADC_TypeDef;

MC_STATIC_ASSERT(offsetof(ADC_TypeDef, DR) == 0x4C, "ADC DR offset");

#define ADC_SR_EOC             (1u << 1)
#define ADC_CR2_ADON           (1u << 0)
#define ADC_CR2_SWSTART        (1u << 30)

/* -------------------------------------------------------------------------- */
/* USART                                                                      */
/* -------------------------------------------------------------------------- */

typedef struct {
    __IO uint32_t SR;
    __IO uint32_t DR;
    __IO uint32_t BRR;
    __IO uint32_t CR1;
    __IO uint32_t CR2;
    __IO uint32_t CR3;
    __IO uint32_t GTPR;
} USART_TypeDef;

MC_STATIC_ASSERT(offsetof(USART_TypeDef, CR1) == 0x0C, "USART CR1 offset");

#define USART_SR_RXNE          (1u << 5)
#define USART_SR_TXE           (1u << 7)
#define USART_CR1_RE           (1u << 2)
#define USART_CR1_TE           (1u << 3)
#define USART_CR1_RXNEIE       (1u << 5)
#define USART_CR1_UE           (1u << 13)

/* -------------------------------------------------------------------------- */
/* IWDG                                                                       */
/* -------------------------------------------------------------------------- */

typedef struct {
    __IO uint32_t KR;
    __IO uint32_t PR;
    __IO uint32_t RLR;
    __IO uint32_t SR;
} IWDG_TypeDef;

MC_STATIC_ASSERT(offsetof(IWDG_TypeDef, RLR) == 0x08, "IWDG RLR offset");

#define IWDG_KR_UNLOCK         0x5555UL
#define IWDG_KR_KICK           0xAAAAUL
#define IWDG_KR_START          0xCCCCUL

/* -------------------------------------------------------------------------- */
/* RCC                                                                        */
/* -------------------------------------------------------------------------- */

typedef struct {
    __IO uint32_t CR;
    __IO uint32_t PLLCFGR;
    __IO uint32_t CFGR;
    __IO uint32_t CIR;
    __IO uint32_t AHB1RSTR;
    __IO uint32_t AHB2RSTR;
    __IO uint32_t AHB3RSTR;
    uint32_t RESERVED0;
    __IO uint32_t APB1RSTR;
    __IO uint32_t APB2RSTR;
    uint32_t RESERVED1[2];
    __IO uint32_t AHB1ENR;
    __IO uint32_t AHB2ENR;
    __IO uint32_t AHB3ENR;
    uint32_t RESERVED2;
    __IO uint32_t APB1ENR;
    __IO uint32_t APB2ENR;
    uint32_t RESERVED3[2];
    __IO uint32_t AHB1LPENR;
    __IO uint32_t AHB2LPENR;
    __IO uint32_t AHB3LPENR;
    uint32_t RESERVED4;
    __IO uint32_t APB1LPENR;
    __IO uint32_t APB2LPENR;
    uint32_t RESERVED5[2];
    __IO uint32_t BDCR;
    __IO uint32_t CSR;
} RCC_TypeDef;

MC_STATIC_ASSERT(offsetof(RCC_TypeDef, AHB1ENR) == 0x30, "RCC AHB1ENR offset");
MC_STATIC_ASSERT(offsetof(RCC_TypeDef, APB1ENR) == 0x40, "RCC APB1ENR offset");
MC_STATIC_ASSERT(offsetof(RCC_TypeDef, CSR) == 0x74, "RCC CSR offset");

#define RCC_CR_HSION           (1u << 0)
#define RCC_CR_HSIRDY          (1u << 1)
#define RCC_AHB1ENR_GPIOAEN    (1u << 0)
#define RCC_AHB1ENR_GPIOBEN    (1u << 1)
#define RCC_AHB1ENR_GPIOCEN    (1u << 2)
#define RCC_AHB1ENR_GPIODEN    (1u << 3)
#define RCC_APB1ENR_TIM3EN     (1u << 1)
#define RCC_APB1ENR_TIM4EN     (1u << 2)
#define RCC_APB1ENR_USART2EN   (1u << 17)
#define RCC_APB2ENR_TIM1EN     (1u << 0)
#define RCC_APB2ENR_ADC1EN     (1u << 8)
#define RCC_CSR_RMVF           (1u << 24)
#define RCC_CSR_IWDGRSTF       (1u << 29)

/* -------------------------------------------------------------------------- */
/* FLASH                                                                      */
/* -------------------------------------------------------------------------- */

typedef struct {
    __IO uint32_t ACR;
    __IO uint32_t KEYR;
    __IO uint32_t OPTKEYR;
    __IO uint32_t SR;
    __IO uint32_t CR;
    __IO uint32_t OPTCR;
} FLASH_TypeDef;

/* -------------------------------------------------------------------------- */
/* Instances                                                                  */
/* -------------------------------------------------------------------------- */

#if defined(MC_HOST_BUILD)
extern GPIO_TypeDef g_hw_gpioa;
extern GPIO_TypeDef g_hw_gpiob;
extern GPIO_TypeDef g_hw_gpioc;
extern GPIO_TypeDef g_hw_gpiod;
extern TIM_TypeDef g_hw_tim1;
extern TIM_TypeDef g_hw_tim3;
extern TIM_TypeDef g_hw_tim4;
extern ADC_TypeDef g_hw_adc1;
extern USART_TypeDef g_hw_usart2;
extern IWDG_TypeDef g_hw_iwdg;
extern RCC_TypeDef g_hw_rcc;
extern FLASH_TypeDef g_hw_flash;
#define GPIOA  (&g_hw_gpioa)
#define GPIOB  (&g_hw_gpiob)
#define GPIOC  (&g_hw_gpioc)
#define GPIOD  (&g_hw_gpiod)
#define TIM1   (&g_hw_tim1)
#define TIM3   (&g_hw_tim3)
#define TIM4   (&g_hw_tim4)
#define ADC1   (&g_hw_adc1)
#define USART2 (&g_hw_usart2)
#define IWDG   (&g_hw_iwdg)
#define RCC    (&g_hw_rcc)
#define FLASH  (&g_hw_flash)
#else
#define GPIOA  ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB  ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC  ((GPIO_TypeDef *)GPIOC_BASE)
#define GPIOD  ((GPIO_TypeDef *)GPIOD_BASE)
#define TIM1   ((TIM_TypeDef *)TIM1_BASE)
#define TIM3   ((TIM_TypeDef *)TIM3_BASE)
#define TIM4   ((TIM_TypeDef *)TIM4_BASE)
#define ADC1   ((ADC_TypeDef *)ADC1_BASE)
#define USART2 ((USART_TypeDef *)USART2_BASE)
#define IWDG   ((IWDG_TypeDef *)IWDG_BASE)
#define RCC    ((RCC_TypeDef *)RCC_BASE)
#define FLASH  ((FLASH_TypeDef *)FLASH_R_BASE)
#endif

#endif /* STM32F407XX_H */
