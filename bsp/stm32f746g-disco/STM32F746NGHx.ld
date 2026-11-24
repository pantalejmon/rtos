/*
 * STM32F746NGH6 — mapa pamięci (RM0385, rozdz. 2.2).
 *
 *   0x0000_0000  ITCM RAM   16 KB   (szybki kod, na razie nieużywany)
 *   0x0800_0000  Flash    1024 KB   (przez magistralę AXIM, z cache L1)
 *   0x2000_0000  DTCM RAM   64 KB  \
 *   0x2001_0000  SRAM1     240 KB   } ciągłe 320 KB — używamy jako jeden region
 *   0x2004_C000  SRAM2      16 KB  /
 *   0xC000_0000  SDRAM       8 MB   (zewnętrzna, wymaga FMC — etap 5)
 *
 * Uwaga na etap 5: DTCM nie jest widoczny dla DMA1, a SRAM1/2 są za cache D.
 * Bufory DMA trzeba będzie albo umieścić w osobnej sekcji, albo utrzymywać
 * spójność cache (SCB_CleanDCache_by_Addr / SCB_InvalidateDCache_by_Addr).
 */

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
    RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 320K
    ITCM  (xrw) : ORIGIN = 0x00000000, LENGTH = 16K
}

REGION_ALIAS("REGION_TEXT", FLASH);
REGION_ALIAS("REGION_RAM",  RAM);

_Min_Heap_Size   = 0x2000;   /*  8 KB na malloc */
_Main_Stack_Size = 0x1000;   /*  4 KB: przerwania + main() przed startem schedulera */

INCLUDE sections.ld
