#include "mc/platform.h"

#define MC_TX_LOG_CAP 2048u
#define MC_RX_CAP 128u

static char s_tx[MC_TX_LOG_CAP];
static unsigned s_tx_len;

#if !defined(MC_HOST_BUILD)
static volatile uint8_t s_rx[MC_RX_CAP];
static volatile uint32_t s_rx_head;
static volatile uint32_t s_rx_tail;

static void rx_push(uint8_t byte)
{
    uint32_t next = (s_rx_head + 1u) % MC_RX_CAP;
    if (next == s_rx_tail) {
        return;
    }
    s_rx[s_rx_head] = byte;
    s_rx_head = next;
}
#endif

void mc_uart_init(uint32_t baud, uint32_t clock_hz)
{
    uint32_t brr;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    (void)RCC->APB1ENR;
    brr = (clock_hz + (baud / 2u)) / baud;
    USART2->BRR = brr;
    USART2->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
#if defined(MC_HOST_BUILD)
    USART2->SR |= USART_SR_TXE;
#else
    s_rx_head = 0u;
    s_rx_tail = 0u;
    /* The hardware buffer is one byte. Capture each byte in the RX interrupt
     * so a command at 115200 is not overwritten between 1 ms control ticks. */
    USART2->CR1 |= USART_CR1_RXNEIE;
    NVIC_SetPriority(USART2_IRQn, 2u);
    NVIC_EnableIRQ(USART2_IRQn);
#endif
    s_tx_len = 0u;
    s_tx[0] = '\0';
}

#if !defined(MC_HOST_BUILD)
void USART2_IRQHandler(void)
{
    if ((USART2->SR & USART_SR_RXNE) != 0u) {
        rx_push((uint8_t)(USART2->DR & 0xFFu));
    }
}
#endif

static void tx_capture(uint8_t byte)
{
    if ((s_tx_len + 2u) >= MC_TX_LOG_CAP) {
        return;
    }
    s_tx[s_tx_len++] = (char)byte;
    s_tx[s_tx_len] = '\0';
}

void mc_uart_write(const char *text)
{
    while (*text != '\0') {
        uint32_t spins = 0u;
        uint8_t byte = (uint8_t)*text++;
        while (((USART2->SR & USART_SR_TXE) == 0u) && (spins < 10000u)) {
            spins++;
        }
        USART2->DR = byte;
        tx_capture(byte);
    }
}

int mc_uart_read(uint8_t *byte_out)
{
#if defined(MC_HOST_BUILD)
    if ((USART2->SR & USART_SR_RXNE) == 0u) {
        return 0;
    }
    *byte_out = (uint8_t)(USART2->DR & 0xFFu);
    /* RXNE is cleared by the DR read on silicon. The host mock needs the store. */
    USART2->SR &= ~USART_SR_RXNE;
    return 1;
#else
    if (s_rx_head == s_rx_tail) {
        return 0;
    }
    *byte_out = s_rx[s_rx_tail];
    s_rx_tail = (s_rx_tail + 1u) % MC_RX_CAP;
    return 1;
#endif
}

const char *mc_uart_tx_log(void)
{
    return s_tx;
}

void mc_uart_tx_log_clear(void)
{
    s_tx_len = 0u;
    s_tx[0] = '\0';
}
