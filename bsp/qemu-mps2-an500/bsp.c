/*
 * bsp.c — QEMU mps2-an500.
 *
 * Konsola: CMSDK UART0 → w QEMU uruchomionym z -nographic trafia na terminal.
 * Dioda:   płytka MPS2 nie ma diody, którą widać w QEMU, więc stan „LD1”
 *          trzymamy w zmiennej (podgląd w debuggerze) i wypisujemy zmiany.
 */
#include "bsp.h"
#include "cmsis_device.h"

uint32_t SystemCoreClock = 25000000u;   /* SYSCLK maszyn MPS2 w QEMU */

static volatile bool led_state;

void bsp_init(void)
{
    CMSDK_UART0->BAUDDIV = 16;              /* minimum dla CMSDK; QEMU i tak nie spowalnia */
    CMSDK_UART0->CTRL    = CMSDK_UART_CTRL_TXEN | CMSDK_UART_CTRL_RXEN;
}

const char *bsp_board_name(void) { return "QEMU mps2-an500 (Cortex-M7)"; }
uint32_t bsp_core_clock_hz(void) { return SystemCoreClock; }

void bsp_led_set(bool on)
{
    if (on == led_state)
        return;
    led_state = on;
    bsp_console_write(on ? "[LD1 *]\r\n" : "[LD1 .]\r\n", 9);
}

void bsp_led_toggle(void)
{
    bsp_led_set(!led_state);
}

void bsp_console_putc(char c)
{
    while (CMSDK_UART0->STATE & CMSDK_UART_STATE_TXFULL) { }
    CMSDK_UART0->DATA = (uint8_t)c;
}

void bsp_console_write(const char *buf, size_t len)
{
    for (size_t i = 0; i < len; i++)
        bsp_console_putc(buf[i]);
}

int bsp_console_getc(void)
{
    if (!(CMSDK_UART0->STATE & CMSDK_UART_STATE_RXFULL))
        return -1;
    return (int)(CMSDK_UART0->DATA & 0xFFu);
}
