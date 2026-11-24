/*
 * port.c — SysTick i priorytety wyjątków systemowych.
 *
 * Etap 2 doda tutaj inicjalizację stosu zadania (port_task_stack_init)
 * i start pierwszego zadania; samo przełączanie trafi do context_switch.S.
 */
#include "port.h"
#include "os.h"

void port_init(uint32_t core_clock_hz, uint32_t tick_hz)
{
    const uint32_t lowest = (1u << __NVIC_PRIO_BITS) - 1u;

    /* PendSV na najniższym priorytecie: przełączenie kontekstu nigdy
     * nie przerwie obsługi innego przerwania w połowie. */
    NVIC_SetPriority(PendSV_IRQn, lowest);
    NVIC_SetPriority(SysTick_IRQn, lowest);

    /* SysTick_Config liczy od 0, stąd w środku jest (ticks - 1); zwraca 1 przy błędzie. */
    if (SysTick_Config(core_clock_hz / tick_hz) != 0u)
    {
        for (;;)
        {
        } /* za duży dzielnik — źle ustawiony zegar albo tick_hz */
    }
}

void SysTick_Handler(void)
{
    os_tick_isr();
}
