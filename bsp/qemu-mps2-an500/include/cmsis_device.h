/*
 * cmsis_device.h — minimalny „nagłówek układu” dla QEMU mps2-an500 (ARM CMSDK, Cortex-M7).
 * Odpowiednik stm32f746xx.h: opis rdzenia + numery przerwań + rejestry peryferiów.
 */
#ifndef CMSIS_DEVICE_H
#define CMSIS_DEVICE_H

#include <stdint.h>

typedef enum {
    /* Wyjątki rdzenia */
    NonMaskableInt_IRQn   = -14,
    HardFault_IRQn        = -13,
    MemoryManagement_IRQn = -12,
    BusFault_IRQn         = -11,
    UsageFault_IRQn       = -10,
    SVCall_IRQn           = -5,
    DebugMonitor_IRQn     = -4,
    PendSV_IRQn           = -2,
    SysTick_IRQn          = -1,
    /* Przerwania CMSDK (AN500) */
    UARTRX0_IRQn          = 0,
    UARTTX0_IRQn          = 1,
    UARTRX1_IRQn          = 2,
    UARTTX1_IRQn          = 3,
    UARTRX2_IRQn          = 4,
    UARTTX2_IRQn          = 5,
    PORT0_ALL_IRQn        = 6,
    PORT1_ALL_IRQn        = 7,
    TIMER0_IRQn           = 8,
    TIMER1_IRQn           = 9,
    DUALTIMER_IRQn        = 10,
    SPI_IRQn              = 11,
    UARTOVF_IRQn          = 12,
    ETHERNET_IRQn         = 13,
    I2S_IRQn              = 14,
    TSC_IRQn              = 15,
} IRQn_Type;

#define __CM7_REV              0x0000U
#define __MPU_PRESENT          1U
#define __NVIC_PRIO_BITS       3U
#define __Vendor_SysTickConfig 0U
#define __FPU_PRESENT          1U
#define __ICACHE_PRESENT       0U    /* QEMU nie emuluje cache */
#define __DCACHE_PRESENT       0U
#define __DTCM_PRESENT         0U
#define __VTOR_PRESENT         1U

#include "core_cm7.h"

extern uint32_t SystemCoreClock;

/* UART z ARM CMSDK (APB UART). */
typedef struct {
    volatile uint32_t DATA;      /* 0x00 */
    volatile uint32_t STATE;     /* 0x04: bit0 TX pełny, bit1 RX pełny */
    volatile uint32_t CTRL;      /* 0x08: bit0 TX włączony, bit1 RX włączony */
    volatile uint32_t INTSTATUS; /* 0x0C: odczyt status / zapis kasowanie */
    volatile uint32_t BAUDDIV;   /* 0x10 */
} CMSDK_UART_TypeDef;

#define CMSDK_UART_STATE_TXFULL  (1u << 0)
#define CMSDK_UART_STATE_RXFULL  (1u << 1)
#define CMSDK_UART_CTRL_TXEN     (1u << 0)
#define CMSDK_UART_CTRL_RXEN     (1u << 1)

#define CMSDK_UART0 ((CMSDK_UART_TypeDef *)0x40004000UL)

#endif
