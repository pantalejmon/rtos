/*
 * sections.ld — wspólny układ sekcji dla wszystkich płytek.
 *
 * Linker script płytki (bsp/<płytka>/*.ld) definiuje MEMORY oraz aliasy:
 *   REGION_ALIAS("REGION_TEXT", <pamięć na kod>)
 *   REGION_ALIAS("REGION_RAM",  <pamięć na dane>)
 * i rozmiary _Min_Heap_Size / _Main_Stack_Size, a potem robi INCLUDE sections.ld.
 *
 * Mapa RAM (od dołu):  .data | .bss | sterta (_heap_start.._heap_end) | ... | stos główny (MSP) przy _estack
 * Stos główny używają: Reset_Handler, main() przed startem schedulera
 * i WSZYSTKIE przerwania. Zadania RTOS-a dostaną własne stosy (etap 2).
 */

ENTRY(Reset_Handler)

_estack = ORIGIN(REGION_RAM) + LENGTH(REGION_RAM);

SECTIONS
{
    /* Tablica wektorów musi być na początku obrazu: słowo 0 = początkowy SP, słowo 1 = Reset_Handler. */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > REGION_TEXT

    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.glue_7) *(.glue_7t)
        *(.eh_frame)
        KEEP(*(.init))
        KEEP(*(.fini))
        . = ALIGN(4);
        _etext = .;
    } > REGION_TEXT

    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } > REGION_TEXT

    .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } > REGION_TEXT
    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } > REGION_TEXT

    /* Konstruktory / destruktory — woła je __libc_init_array() w Reset_Handler. */
    .preinit_array :
    {
        PROVIDE_HIDDEN(__preinit_array_start = .);
        KEEP(*(.preinit_array*))
        PROVIDE_HIDDEN(__preinit_array_end = .);
    } > REGION_TEXT
    .init_array :
    {
        PROVIDE_HIDDEN(__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array*))
        PROVIDE_HIDDEN(__init_array_end = .);
    } > REGION_TEXT
    .fini_array :
    {
        PROVIDE_HIDDEN(__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array*))
        PROVIDE_HIDDEN(__fini_array_end = .);
    } > REGION_TEXT

    /* .data: adres wykonania w RAM, wartości początkowe zapisane we flashu (kopiuje Reset_Handler). */
    _sidata = LOADADDR(.data);
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > REGION_RAM AT> REGION_TEXT

    /* .bss: zerowane przez Reset_Handler. */
    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        __bss_start__ = _sbss;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
        __bss_end__ = _ebss;
    } > REGION_RAM

    /* Sterta dla malloc (newlib) — zakres sprawdza _sbrk() w syscalls.c. */
    .heap (NOLOAD) :
    {
        . = ALIGN(8);
        PROVIDE(end = .);
        PROVIDE(_end = .);
        _heap_start = .;
        . = . + _Min_Heap_Size;
        . = ALIGN(8);
        _heap_end = .;
    } > REGION_RAM

    /* Rezerwacja na stos główny: linker zgłosi błąd, jeśli zabraknie RAM-u. */
    .main_stack (NOLOAD) :
    {
        . = ALIGN(8);
        . = . + _Main_Stack_Size;
        . = ALIGN(8);
    } > REGION_RAM

    _main_stack_limit = _estack - _Main_Stack_Size;

    .ARM.attributes 0 : { *(.ARM.attributes) }
}
