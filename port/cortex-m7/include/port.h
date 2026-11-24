/*
 * port.h — interfejs warstwy rdzenia (Cortex-M7) dla jądra.
 *
 * Jądro woła wyłącznie te funkcje i nie dotyka rejestrów bezpośrednio.
 * Przy przejściu na inny rdzeń (np. Cortex-M33) piszemy nowy katalog port/.
 */
#ifndef PORT_H
#define PORT_H

#include <stdint.h>

#include "cmsis_device.h"

/* Uruchamia SysTick z częstotliwością tick_hz i ustawia priorytety
 * wyjątków systemowych (PendSV i SysTick najniższe). */
void port_init(uint32_t core_clock_hz, uint32_t tick_hz);

/*
 * Sekcja krytyczna.
 * Na razie przez PRIMASK (wyłącza wszystkie przerwania). W etapie 4 przejdziemy
 * na BASEPRI, żeby przerwania o wysokim priorytecie, które nie wołają API RTOS-a,
 * działały nawet w sekcji krytycznej.
 *
 *   uint32_t s = port_enter_critical();
 *   ... kilka instrukcji ...
 *   port_exit_critical(s);
 */
static inline uint32_t port_enter_critical(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

static inline void port_exit_critical(uint32_t primask)
{
    __set_PRIMASK(primask);
}

/* Czekaj na przerwanie (zadanie idle). */
static inline void port_wait_for_interrupt(void)
{
    __DSB();
    __WFI();
}

/* Zgłoś potrzebę przełączenia kontekstu — PendSV wykona się, gdy skończą się
 * wszystkie inne przerwania (etap 2). */
static inline void port_request_context_switch(void)
{
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
    __DSB();
    __ISB();
}

#endif /* PORT_H */
