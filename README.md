# rtos

Własny system czasu rzeczywistego dla Cortex-M7, pisany od zera w celach edukacyjnych.

- **Płytka docelowa:** STM32F746G-DISCO (później STM32H7S78-DK)
- **Bez płytki:** QEMU `mps2-an500` (emulowany Cortex-M7)
- **Narzędzia:** CLion, CMake + Ninja, `arm-none-eabi-gcc`, OpenOCD

## Szybki start

```bash
sudo ./tools/setup-host.sh                    # jednorazowo: toolchain, OpenOCD, QEMU

cmake --preset qemu-debug
cmake --build --preset qemu-debug -t qemu     # zbuduj i uruchom w QEMU (wyjście: Ctrl+A, X)

cmake --preset f746-debug
cmake --build --preset f746-debug -t flash    # zbuduj i wgraj na płytkę
```

W CLion: *File → Open* na katalogu projektu, a presety pojawią się jako profile CMake. Szczegóły, w tym konfiguracje debugowania, są w [docs/02](docs/02-srodowisko-i-pierwszy-start.md#4-clion).

## Warstwy

| Katalog | Zawartość | Zależy od płytki? |
|---|---|---|
| `app/` | aplikacja, zadania | nie |
| `kernel/` | scheduler, TCB, synchronizacja — czyste C | nie |
| `port/cortex-m7/` | SysTick, PendSV, SVC, sekcje krytyczne, HardFault | nie (tylko od rdzenia) |
| `bsp/<płytka>/` | startup, zegary, piny, UART, linker script | **tak** |

## Dokumentacja

1. [01 — Od pętli `while(1)` do RTOS-a](docs/01-od-petli-do-rtosa.md): koncepcja, przełączanie kontekstu na Cortex-M7, plan projektu
2. [02 — Środowisko i pierwszy start](docs/02-srodowisko-i-pierwszy-start.md): narzędzia, CLion, startup, linker script, zegar 216 MHz
3. 03 — Przełączanie kontekstu *(następny)*
4. 04 — Scheduler z priorytetami i czasem
5. 05 — Synchronizacja

## Stan

- [x] Szablon projektu, CMake, presety, QEMU
- [ ] Etap 1 na płytce: miganie LD1 + `printf` przez ST-Link (kod gotowy, czeka na płytkę)
- [ ] Etap 2: przełączanie kontekstu

## Licencje

Nagłówki w `third_party/`: CMSIS 6 (ARM) i cmsis-device-f7 (STMicroelectronics), oba na licencji Apache-2.0. Pliki licencji leżą w ich katalogach.
