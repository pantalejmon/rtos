# CLAUDE.md

Edukacyjny projekt: własny RTOS na Cortex-M7, pisany od zera (bez FreeRTOS, bez HAL).
Użytkownik uczy się — kod ma być czytelny i skomentowany po polsku, z odwołaniami do docs/.

## Budowanie

- `cmake --preset qemu-debug && cmake --build --preset qemu-debug -t qemu` — test w QEMU (bez płytki)
- `cmake --preset f746-debug && cmake --build --preset f746-debug [-t flash]` — STM32F746G-DISCO
- Test nieinteraktywny w QEMU: `timeout 5 qemu-system-arm -M mps2-an500 -nographic -kernel build/qemu-debug/app/rtos.elf`

## Zasady architektury

- Warstwy: app → kernel → port → bsp. `kernel/` nie dotyka rejestrów (tylko `port.h`), `port/` nie zna płytki (tylko `cmsis_device.h`).
- Nowa płytka = nowy katalog w `bsp/` z `board.cmake`, `CMakeLists.txt`, `include/cmsis_device.h`, `include/irq_*.inc`, linker scriptem.
- Biblioteki CMake są `OBJECT` (słabe handlery przerwań + archiwa .a = zgubione handlery).
- Bez HAL — rejestry przez nazwy z CMSIS (`RCC->CR`, `USART1->ISR`).
- IDE: CLion (formatowanie: klamry w nowej linii).

## Dokumentacja

`docs/NN-*.md` — po polsku, jeden dokument na etap. Plan etapów: docs/01, sekcja „Plan projektu”.
