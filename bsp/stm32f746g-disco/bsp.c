/*
 * bsp.c — STM32F746G-DISCO, bez HAL-a, bezpośrednio na rejestrach (nazwy z CMSIS).
 *
 * Zasoby płytki (UM1907):
 *   HSE   25 MHz kwarc X2
 *   LD1   zielona dioda, PI1 (aktywna stanem wysokim)
 *   VCP   USART1: TX = PA9, RX = PB7 (AF7) → ST-Link → /dev/ttyACM0
 */
#include "bsp.h"
#include "cmsis_device.h"

#define CONSOLE_BAUD 115200u

/* Zmienna CMSIS (zadeklarowana w system_stm32f7xx.h). Po resecie rdzeń działa na HSI 16 MHz. */
uint32_t SystemCoreClock = 16000000u;

static uint32_t apb2_clock_hz = 16000000u;

/*
 * SYSCLK = HSE / M * N / P = 25 MHz / 25 * 432 / 2 = 216 MHz
 * PLL48  = 25 MHz / 25 * 432 / 9 = 48 MHz (USB, SDMMC — na później)
 * AHB 216 MHz, APB1 = /4 = 54 MHz, APB2 = /2 = 108 MHz
 *
 * 216 MHz wymaga trybu over-drive regulatora i 7 cykli oczekiwania flasha (RM0385, tab. 7).
 */
static void clock_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    (void)RCC->APB1ENR;                                  /* odczyt = opóźnienie po włączeniu zegara */
    PWR->CR1 |= PWR_CR1_VOS;                             /* skala napięcia 1 (najwyższa wydajność) */

    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY)) { }

    RCC->PLLCFGR = (RCC->PLLCFGR & ~(RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLP |
                                     RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLQ))
                 | (25u  << RCC_PLLCFGR_PLLM_Pos)
                 | (432u << RCC_PLLCFGR_PLLN_Pos)
                 | (0u   << RCC_PLLCFGR_PLLP_Pos)       /* 0 oznacza P = 2 */
                 | (9u   << RCC_PLLCFGR_PLLQ_Pos)
                 | RCC_PLLCFGR_PLLSRC_HSE;
    RCC->CR |= RCC_CR_PLLON;

    PWR->CR1 |= PWR_CR1_ODEN;
    while (!(PWR->CSR1 & PWR_CSR1_ODRDY)) { }
    PWR->CR1 |= PWR_CR1_ODSWEN;
    while (!(PWR->CSR1 & PWR_CSR1_ODSWRDY)) { }

    while (!(RCC->CR & RCC_CR_PLLRDY)) { }

    FLASH->ACR = FLASH_ACR_LATENCY_7WS | FLASH_ACR_PRFTEN | FLASH_ACR_ARTEN;
    while ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_7WS) { }

    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2))
              | RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV4 | RCC_CFGR_PPRE2_DIV2;

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) { }

    SystemCoreClock = 216000000u;
    apb2_clock_hz   = 108000000u;
}

/* Cache L1 rdzenia M7. Bez nich kod z flasha przy 216 MHz jest kilka razy wolniejszy.
 * D-cache będzie wymagać uwagi przy DMA (etap 5). */
static void cache_init(void)
{
    SCB_EnableICache();
    SCB_EnableDCache();
}

static void gpio_set_af(GPIO_TypeDef *port, uint32_t pin, uint32_t af)
{
    port->MODER   = (port->MODER & ~(3u << (pin * 2))) | (2u << (pin * 2));   /* tryb alternatywny */
    port->OSPEEDR |= (2u << (pin * 2));                                        /* szybkie zbocza */
    port->AFR[pin / 8] = (port->AFR[pin / 8] & ~(0xFu << ((pin % 8) * 4))) | (af << ((pin % 8) * 4));
}

static void led_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOIEN;
    (void)RCC->AHB1ENR;
    GPIOI->MODER = (GPIOI->MODER & ~(3u << (1 * 2))) | (1u << (1 * 2));       /* PI1 wyjście */
    GPIOI->BSRR  = 1u << (1 + 16);                                             /* zgaszona */
}

static void console_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;                /* zegar USART1 = PCLK2 (domyślnie) */
    (void)RCC->APB2ENR;

    gpio_set_af(GPIOA, 9, 7);                            /* PA9 = USART1_TX */
    gpio_set_af(GPIOB, 7, 7);                            /* PB7 = USART1_RX */

    USART1->CR1 = 0;
    USART1->BRR = (apb2_clock_hz + CONSOLE_BAUD / 2) / CONSOLE_BAUD;   /* oversampling 16 */
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void bsp_init(void)
{
    clock_init();
    cache_init();
    led_init();
    console_init();
}

const char *bsp_board_name(void) { return "STM32F746G-DISCO"; }
uint32_t bsp_core_clock_hz(void) { return SystemCoreClock; }

void bsp_led_set(bool on)
{
    GPIOI->BSRR = on ? (1u << 1) : (1u << (1 + 16));     /* BSRR: atomowe ustawienie/kasowanie */
}

void bsp_led_toggle(void)
{
    bsp_led_set(!(GPIOI->ODR & (1u << 1)));
}

void bsp_console_putc(char c)
{
    while (!(USART1->ISR & USART_ISR_TXE)) { }
    USART1->TDR = (uint8_t)c;
}

void bsp_console_write(const char *buf, size_t len)
{
    for (size_t i = 0; i < len; i++)
        bsp_console_putc(buf[i]);
}

int bsp_console_getc(void)
{
    uint32_t isr = USART1->ISR;
    if (isr & USART_ISR_ORE)
        USART1->ICR = USART_ICR_ORECF;                   /* overrun: bajt zginął, czyścimy flagę */
    if (!(isr & USART_ISR_RXNE))
        return -1;
    return (int)(USART1->RDR & 0xFFu);
}
