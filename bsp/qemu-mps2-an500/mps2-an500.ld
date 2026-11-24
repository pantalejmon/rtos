/*
 * QEMU mps2-an500 — mapa pamięci (hw/arm/mps2.c w QEMU):
 *   0x0000_0000  ZBT SSRAM1   4 MB  — tu QEMU ładuje ELF i stąd czyta tablicę wektorów
 *   0x2000_0000  ZBT SSRAM2/3 4 MB  — dane
 *
 * Rozmiary celowo ograniczone do 1 MB / 320 KB, żeby program, który zmieści się
 * w QEMU, zmieścił się też w STM32F746.
 */

MEMORY
{
    CODE (rx)  : ORIGIN = 0x00000000, LENGTH = 1024K
    RAM  (xrw) : ORIGIN = 0x20000000, LENGTH = 320K
}

REGION_ALIAS("REGION_TEXT", CODE);
REGION_ALIAS("REGION_RAM",  RAM);

_Min_Heap_Size   = 0x2000;
_Main_Stack_Size = 0x1000;

INCLUDE sections.ld
