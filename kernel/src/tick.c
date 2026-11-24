/*
 * tick.c — czas systemowy.
 *
 * Etap 3 dopisze tu budzenie zadań z listy śpiących i zgłaszanie
 * przełączenia kontekstu (port_request_context_switch()).
 */
#include "os.h"

static volatile uint32_t tick_count;

void os_tick_isr(void)
{
    tick_count++;
}

uint32_t os_ticks(void)
{
    return tick_count;      /* odczyt 32-bitowego słowa jest atomowy na Cortex-M */
}
