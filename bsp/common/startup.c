/*
 * startup.c — tablica wektorów i Reset_Handler, wspólne dla płytek z Cortex-M7.
 *
 * Po resecie rdzeń Cortex-M sam:
 *   1. ładuje MSP ze słowa 0 tablicy wektorów (_estack z linker scriptu),
 *   2. skacze pod adres ze słowa 1 (Reset_Handler).
 * Nie ma żadnego „BIOS-u” — wszystko dalej robimy tutaj, w C.
 *
 * Lista przerwań zależy od układu, dlatego płytka dostarcza dwa pliki:
 *   irq_handlers.inc — deklaracje słabych handlerów (WEAK_ALIAS void X_IRQHandler(void);)
 *   irq_vectors.inc  — kolejne wpisy tablicy od wektora 16 w górę
 *
 * Każdy handler jest „weak” i domyślnie wskazuje na Default_Handler. Żeby obsłużyć
 * przerwanie, wystarczy zdefiniować w dowolnym pliku funkcję o tej samej nazwie.
 */
#include <stdint.h>

#include "cmsis_device.h"

typedef void (*vector_t)(void);

/* Symbole z linker scriptu (bsp/common/sections.ld). */
extern uint32_t _estack;
extern uint32_t _sidata, _sdata, _edata;
extern uint32_t _sbss, _ebss;

extern int main(void);
extern void __libc_init_array(void);

void Reset_Handler(void);
void Default_Handler(void);

#define WEAK_ALIAS __attribute__((weak, alias("Default_Handler")))

/* Wyjątki rdzenia — wspólne dla wszystkich Cortex-M7. */
WEAK_ALIAS void NMI_Handler(void);
WEAK_ALIAS void HardFault_Handler(void);
WEAK_ALIAS void MemManage_Handler(void);
WEAK_ALIAS void BusFault_Handler(void);
WEAK_ALIAS void UsageFault_Handler(void);
WEAK_ALIAS void SVC_Handler(void);
WEAK_ALIAS void DebugMon_Handler(void);
WEAK_ALIAS void PendSV_Handler(void);
WEAK_ALIAS void SysTick_Handler(void);

/* Przerwania układu. */
#include "irq_handlers.inc"

__attribute__((section(".isr_vector"), used))
const vector_t vector_table[] = {
    (vector_t)&_estack,         /*  0: początkowy MSP */
    Reset_Handler,              /*  1 */
    NMI_Handler,                /*  2 */
    HardFault_Handler,          /*  3 */
    MemManage_Handler,          /*  4 */
    BusFault_Handler,           /*  5 */
    UsageFault_Handler,         /*  6 */
    0, 0, 0, 0,                 /*  7–10: zarezerwowane */
    SVC_Handler,                /* 11: instrukcja svc — start pierwszego zadania (etap 2) */
    DebugMon_Handler,           /* 12 */
    0,                          /* 13: zarezerwowane */
    PendSV_Handler,             /* 14: przełączanie kontekstu (etap 2) */
    SysTick_Handler,            /* 15: tick systemowy */
#include "irq_vectors.inc"
};

void Reset_Handler(void)
{
#if defined(__FPU_USED) && (__FPU_USED == 1U)
    /* Kompilujemy z -mfloat-abi=hard, więc FPU musi być włączone, zanim
     * jakikolwiek kod C dotknie rejestrów s0–s31. CP10 i CP11 = pełny dostęp. */
    SCB->CPACR |= (3UL << 20) | (3UL << 22);
    __DSB();
    __ISB();
#endif

    SCB->VTOR = (uint32_t)vector_table;

    /* .data: kopiujemy wartości początkowe z flasha do RAM. */
    for (uint32_t *src = &_sidata, *dst = &_sdata; dst < &_edata; )
        *dst++ = *src++;

    /* .bss: zerujemy. */
    for (uint32_t *dst = &_sbss; dst < &_ebss; )
        *dst++ = 0;

    /* Konstruktory (atrybut constructor, C++) i inicjalizacja newliba. */
    __libc_init_array();

    main();

    for (;;) { }                /* main() nie powinien wrócić */
}

/*
 * Tu trafia każde przerwanie bez własnego handlera.
 * W debuggerze: rejestr IPSR (najniższe bity xPSR) = numer wyjątku,
 * czyli 16 + numer IRQ. To pierwsza rzecz do sprawdzenia, gdy program „wisi” tutaj.
 */
void Default_Handler(void)
{
    volatile uint32_t active_exception = __get_IPSR();
    (void)active_exception;
    for (;;) { }
}
