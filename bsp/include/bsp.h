/*
 * bsp.h — interfejs warstwy płytki.
 *
 * Ten sam nagłówek dla każdej płytki; implementacja w bsp/<płytka>/bsp.c.
 * Jądro (kernel/) NIE korzysta z tego pliku — używa go tylko aplikacja
 * i ewentualnie sterowniki. Dzięki temu zmiana płytki = nowy katalog w bsp/.
 */
#ifndef BSP_H
#define BSP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Zegary, cache, piny, LED, UART konsoli. Wołać jako pierwsze w main(). */
void bsp_init(void);

/* Nazwa płytki do wypisania w bannerze. */
const char *bsp_board_name(void);

/* Częstotliwość rdzenia po bsp_init() (np. 216 MHz na F746). */
uint32_t bsp_core_clock_hz(void);

/* Dioda użytkownika (LD1 na F746G-DISCO). */
void bsp_led_set(bool on);
void bsp_led_toggle(void);

/* Konsola UART (115200 8N1). Na F746G-DISCO to wirtualny port COM ST-Linka
 * (/dev/ttyACM0), w QEMU — standardowe wyjście terminala. */
void bsp_console_putc(char c);
void bsp_console_write(const char *buf, size_t len);
int  bsp_console_getc(void);   /* zwraca bajt albo -1, gdy nic nie przyszło; nie blokuje */

#endif /* BSP_H */
