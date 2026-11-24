/*
 * fault.c — HardFault, który mówi, co się stało.
 *
 * Najczęstszy błąd przy pisaniu przełączania kontekstu to źle przygotowany stos
 * zadania (pomyłka o jedno słowo). Kończy się to HardFaultem. Ten handler zapisuje
 * ramkę wyjątku i rejestry statusu do zmiennej `port_fault`, a jeśli podłączony
 * jest debugger — zatrzymuje się na breakpoincie.
 *
 * W debuggerze obejrzyj `port_fault`:
 *   pc   — instrukcja, która spowodowała błąd (Ctrl+klik w CLion → disassembly)
 *   lr   — skąd była wołana funkcja
 *   cfsr — przyczyna (UFSR/BFSR/MMFSR, opis w PM0253 rozdz. 4.3.10)
 *   hfsr — bit FORCED = eskalacja z innego błędu
 *   msp_used — 1 = błąd w przerwaniu/jądrze, 0 = w zadaniu (PSP)
 */
#include "port.h"

typedef struct {
    uint32_t r0, r1, r2, r3, r12, lr, pc, xpsr;
    uint32_t cfsr, hfsr, mmfar, bfar;
    uint32_t exc_return;
    uint32_t msp_used;
} port_fault_t;

volatile port_fault_t port_fault;

void port_hardfault_c(const uint32_t *frame, uint32_t exc_return);

__attribute__((naked)) void HardFault_Handler(void)
{
    __asm volatile(
        "tst   lr, #4            \n"    /* bit 2 EXC_RETURN: 0 = MSP, 1 = PSP */
        "ite   eq                \n"
        "mrseq r0, msp           \n"
        "mrsne r0, psp           \n"
        "mov   r1, lr            \n"
        "b     port_hardfault_c  \n"
    );
}

__attribute__((used)) void port_hardfault_c(const uint32_t *frame, uint32_t exc_return)
{
    port_fault.r0   = frame[0];
    port_fault.r1   = frame[1];
    port_fault.r2   = frame[2];
    port_fault.r3   = frame[3];
    port_fault.r12  = frame[4];
    port_fault.lr   = frame[5];
    port_fault.pc   = frame[6];
    port_fault.xpsr = frame[7];
    port_fault.cfsr  = SCB->CFSR;
    port_fault.hfsr  = SCB->HFSR;
    port_fault.mmfar = SCB->MMFAR;
    port_fault.bfar  = SCB->BFAR;
    port_fault.exc_return = exc_return;
    port_fault.msp_used   = (exc_return & 4u) ? 0u : 1u;

    if (DCB->DHCSR & DCB_DHCSR_C_DEBUGEN_Msk)
        __BKPT(0);                      /* debugger podłączony: zatrzymaj się tutaj */

    for (;;) { }
}
