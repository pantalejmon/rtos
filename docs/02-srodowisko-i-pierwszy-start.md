# 02 — Środowisko i pierwszy start

Ten dokument opisuje narzędzia, strukturę repozytorium i to, co dzieje się od resetu do pierwszego `printf`. Kończy się etapem 1 z planu: działa startup, linker script, zegar 216 MHz, SysTick, dioda LD1 i `printf` przez UART.

Dopóki nie ma płytki, ten sam kod uruchamiamy w **QEMU** na maszynie `mps2-an500`, czyli emulowanym Cortex-M7. Jądro i port są wspólne, zmienia się tylko katalog `bsp/`.

## 1. Narzędzia

| Narzędzie | Do czego | Pakiet Ubuntu |
|---|---|---|
| `arm-none-eabi-gcc` | kompilator C dla Cortex-M | `gcc-arm-none-eabi` |
| newlib / newlib-nano | biblioteka C (`printf`, `memcpy`, `malloc`) | `libnewlib-arm-none-eabi` |
| `objcopy`, `size`, `objdump` | `.elf` → `.bin`/`.hex`, rozmiar sekcji, disassembly | `binutils-arm-none-eabi` |
| CMake + Ninja | budowanie, presety dla CLion | `cmake`, `ninja-build` |
| OpenOCD | wgrywanie i serwer GDB przez ST-Link | `openocd` |
| stlink-tools | `st-info --probe`: szybki test, czy płytka odpowiada | `stlink-tools` |
| QEMU | emulator Cortex-M7 do pracy bez płytki | `qemu-system-arm` |
| `gdb-multiarch` | debugger ARM z linii poleceń (CLion ma własny) | `gdb-multiarch` |
| picocom | terminal szeregowy `/dev/ttyACM0` | `picocom` |

Instalacja, jednorazowo:

```bash
sudo ./tools/setup-host.sh
```

Skrypt instaluje pakiety, przeładowuje reguły udev (dostęp do ST-Linka bez roota) i dodaje użytkownika do grupy `dialout` (dostęp do `/dev/ttyACM0`).

> **STM32CubeCLT zamiast pakietów z Ubuntu.** Dokument 01 wspominał CubeCLT. Do budowania wystarczają pakiety systemowe. CubeCLT przyda się później, bo zawiera STM32CubeProgrammer i pliki SVD. Ściąga się go ze strony ST po zalogowaniu. Żeby budować jego kompilatorem, podaj `-DARM_TOOLCHAIN_DIR=/opt/st/stm32cubeclt_<wersja>/GNU-tools-for-STM32/bin`.

## 2. Struktura repozytorium

```
rtos/
├── CMakeLists.txt            główny plik, wybór płytki przez RTOS_BOARD
├── CMakePresets.json         qemu-debug, f746-debug, f746-release (CLion = profile)
├── cmake/arm-none-eabi.cmake toolchain file
├── app/                      aplikacja (main.c) + cele flash / qemu
├── kernel/                   jądro — czyste C, bez rejestrów
│   ├── include/os.h          publiczne API (z planem kolejnych etapów)
│   ├── include/os_config.h   OS_TICK_HZ, limity
│   └── src/tick.c            licznik ticków
├── port/cortex-m7/           rdzeń: SysTick, priorytety, sekcje krytyczne, HardFault
├── bsp/
│   ├── include/bsp.h         interfejs płytki (wspólny)
│   ├── common/               startup.c, syscalls.c, sections.ld (wspólne)
│   ├── stm32f746g-disco/     zegary 216 MHz, LD1, USART1, linker script, openocd.cfg
│   └── qemu-mps2-an500/      UART CMSDK, „dioda” w konsoli, linker script
├── third_party/              nagłówki CMSIS 6 (ARM) i STM32F7 (ST), licencja Apache-2.0
├── tools/                    setup-host.sh, qemu.sh, flash.sh, serial.sh
├── .run/                     gotowe konfiguracje uruchomieniowe CLion
└── docs/                     ta dokumentacja
```

Zależności między warstwami (CMake):

```
app ──► kernel ──► port ──► cmsis_device (nagłówki CMSIS + cmsis_device.h płytki)
  └───► bsp  ───────────────►┘
```

`port` dołącza `cmsis_device.h`, a ten plik dostarcza płytka. Na F746 zawiera on `stm32f746xx.h`, w QEMU minimalny opis rdzenia. Dzięki temu port nie wie, na jakiej płytce działa.

Wszystkie biblioteki są typu `OBJECT`, a nie `STATIC`. Handlery przerwań w startupie są słabe (`weak`), a linker nie wyciąga z archiwum `.a` pliku tylko po to, żeby nadpisać symbol słaby. Z bibliotekami statycznymi `SysTick_Handler` z portu po cichu by przepadł.

Pułapka bibliotek `OBJECT`: zależność przechodnia przekazuje tylko nagłówki, a nie pliki `.o`. Dlatego `app/CMakeLists.txt` wymienia wprost wszystkie trzy biblioteki (`kernel port bsp`). Bez `port` linker zgłasza `undefined reference to 'port_init'`.

## 3. Budowanie z linii poleceń

```bash
cmake --preset qemu-debug                    # konfiguracja → build/qemu-debug/
cmake --build --preset qemu-debug            # kompilacja → build/qemu-debug/app/rtos.elf
cmake --build --preset qemu-debug -t qemu    # uruchom w QEMU (wyjście: Ctrl+A, X)

cmake --preset f746-debug
cmake --build --preset f746-debug            # → build/f746-debug/app/rtos.{elf,bin,hex,map}
cmake --build --preset f746-debug -t flash   # wgraj przez ST-Link
```

Po każdym linkowaniu widać zajętość pamięci (`--print-memory-usage`) i rozmiary sekcji (`size`). Plik `rtos.map` pokazuje, która funkcja ile zajmuje i gdzie leży.

## 4. CLion

### Otwarcie projektu

1. *File → Open* → katalog `rtos`. CLion wykryje `CMakePresets.json`.
2. *Settings → Build, Execution, Deployment → CMake*: profile `qemu-debug`, `f746-debug` i `f746-release` pochodzą z presetów. Włącz te, których używasz (checkbox *Enable profile*). Domyślny profil `Debug` (z kompilatorem hosta) możesz wyłączyć, bo dla tego projektu nie zadziała.
3. Toolchain nie wymaga ustawień. Kompilator ARM wskazuje `cmake/arm-none-eabi.cmake`, a debugger *Bundled GDB* w CLion obsługuje ARM.

Profil wybierasz z listy obok przycisku Build. Cel `rtos` buduje firmware. Cele `qemu` i `flash` po zbudowaniu uruchamiają QEMU albo wgrywają program na płytkę.

### Uruchamianie bez płytki (QEMU)

Gotowe konfiguracje w katalogu `.run/` są typu *CMake Application* bez pliku wykonywalnego. Zielony trójkąt (Shift+F10) buduje wtedy tylko wskazany cel CMake:

| Konfiguracja | Cel CMake | Profil | Co robi |
|---|---|---|---|
| **Build: rtos** | `rtos` | `qemu-debug` / `f746-debug` | buduje `rtos.elf` (oraz `.bin`, `.hex`) |
| **QEMU: run** | `qemu` | `qemu-debug` | buduje `rtos`, potem uruchamia QEMU. Wyjście programu jest w oknie *Build*, a QEMU zatrzymujesz czerwonym kwadratem |
| **F746: flash** | `flash` | `f746-debug` | buduje `rtos` i wgrywa przez ST-Link |

Profil wybierasz z listy obok konfiguracji. Cel `qemu` istnieje tylko w profilu QEMU, a `flash` tylko w profilach F746. Jeśli chcesz interaktywnej konsoli (Ctrl+A, X), uruchom `tools/qemu.sh` w terminalu.

Do debugowania utwórz konfigurację **Embedded GDB Server** (*Run → Edit Configurations → + → Embedded GDB Server*):

| Pole | Wartość |
|---|---|
| Name | `QEMU: debug` |
| Target | `rtos` |
| Executable binary | `rtos` |
| Debugger | Bundled GDB |
| Upload executable | **Never** (QEMU sam ładuje ELF) |
| 'target remote' args | `tcp:localhost:1234` |
| GDB Server | `qemu-system-arm` (pełna ścieżka: `/usr/bin/qemu-system-arm`) |
| GDB Server args | `-M mps2-an500 -nographic -kernel $CMakeCurrentProductFile$ -S -gdb tcp::1234` |

Profil CMake musi być `qemu-debug`. Teraz Debug (Shift+F9) uruchamia QEMU zatrzymane na resecie, podłącza GDB i zatrzymuje się na breakpointach. Wyjście programu widać w zakładce konsoli serwera GDB.

Wariant w dwóch krokach: najpierw uruchom gotową konfigurację **QEMU: gdb server** (QEMU wypisze `czeka na GDB: localhost:1234` i zatrzyma się na resecie). Potem uruchom w trybie Debug konfigurację **Remote Debug** (*+ → Remote Debug*) z takimi polami:

| Pole | Wartość |
|---|---|
| Name | `QEMU: attach` |
| Debugger | Bundled GDB |
| 'target remote' args | `tcp:localhost:1234` |
| Symbol file | `/home/jan/projekty/rtos/build/qemu-debug/app/rtos.elf` (ścieżka bezwzględna, wybierz przyciskiem …) |
| Sysroot | puste |

Ten wariant nie buduje programu przed startem, więc najpierw zbuduj cel `rtos` w profilu `qemu-debug`.

Alternatywa bez CLion: `tools/qemu.sh --debug` w jednym terminalu i `gdb-multiarch build/qemu-debug/app/rtos.elf -ex "target remote :1234"` w drugim.

### Uruchamianie na płytce (gdy przyjdzie)

Konfiguracja **OpenOCD Download & Run** (*+ → OpenOCD Download & Run*):

| Pole | Wartość |
|---|---|
| Name | `F746: debug` |
| Target / Executable | `rtos` |
| Debugger | Bundled GDB |
| Board config file | `bsp/stm32f746g-disco/openocd.cfg` |
| Download | If updated |
| Reset | Init |

Profil CMake: `f746-debug`. Konsolę UART otwiera gotowa konfiguracja **Płytka: konsola UART** (`tools/serial.sh`, picocom 115200).

### Podgląd rejestrów peryferiów (opcjonalnie)

CLion potrafi pokazać rejestry RCC, GPIO czy USART z nazwami pól w zakładce *Peripherals* debuggera. Potrzebuje do tego pliku SVD (`STM32F746.svd`, jest w STM32CubeCLT albo w repozytorium `cmsis-svd`). W trakcie debugowania: *Peripherals → Load .svd file*.

## 5. Od resetu do `main()`

### Co robi rdzeń sam

Po resecie Cortex-M7 czyta dwa pierwsze słowa z adresu 0. Na F746 flash 0x0800_0000 jest tam widoczny dzięki aliasowi BOOT0=0.

1. Słowo 0 trafia do MSP. To `_estack`, czyli koniec RAM.
2. Słowo 1 to adres `Reset_Handler`, pod który rdzeń skacze.

Nie ma żadnego BIOS-u, więc od tego miejsca wszystko robi nasz kod.

### `bsp/common/startup.c`

- **Tablica wektorów** `vector_table[]` w sekcji `.isr_vector`. Najpierw jest 16 wyjątków rdzenia, potem przerwania układu z plików `irq_handlers.inc`/`irq_vectors.inc` płytki. Dla F746 są one wygenerowane z `IRQn_Type` w `stm32f746xx.h` (98 wektorów).
- **Słabe handlery.** Każdy handler jest aliasem `Default_Handler`. Żeby obsłużyć przerwanie, wystarczy w dowolnym pliku zdefiniować funkcję o tej nazwie, np. `void USART1_IRQHandler(void)`.
- **`Reset_Handler`** wykonuje kolejno:
  1. włącza FPU (CP10/CP11 w `SCB->CPACR`). Musi to być pierwsza czynność, bo kompilujemy z `-mfloat-abi=hard` i kompilator może użyć rejestrów `s0–s31` w dowolnym kodzie,
  2. ustawia `VTOR` na tablicę wektorów,
  3. kopiuje `.data` z flasha do RAM,
  4. zeruje `.bss`,
  5. wywołuje `__libc_init_array()` (konstruktory),
  6. wywołuje `main()`.
- **`Default_Handler`** zapisuje `IPSR`, czyli numer aktywnego wyjątku (16 + numer IRQ). Gdy program „wisi” w `Default_Handler`, w debuggerze sprawdź zmienną `active_exception`.

### Linker script

Plik płytki (`STM32F746NGHx.ld`) definiuje tylko `MEMORY` i rozmiary sterty i stosu. Układ sekcji jest wspólny i leży w `bsp/common/sections.ld`:

```
FLASH 0x0800_0000                          RAM 0x2000_0000 (320 KB)
┌─────────────┐                            ┌──────────────┐
│ .isr_vector │                            │ .data        │◄─┐ kopiowane przez Reset_Handler
│ .text       │                            │ .bss         │  │ zerowane przez Reset_Handler
│ .rodata     │                            │ sterta 8 KB  │  │ malloc (_sbrk)
│ init_array  │                            │   ...wolne   │  │
│ wartości    │────────────────────────────┼──────────────┼──┘
│ .data (LMA) │                            │ stos MSP 4KB │ ◄─ _estack (koniec RAM)
└─────────────┘                            └──────────────┘
```

Kluczowe pojęcie to **LMA i VMA**. Zmienna `int x = 5;` ma adres w RAM (VMA), ale wartość 5 musi przetrwać wyłączenie zasilania, więc leży we flashu (LMA). Zapis `> REGION_RAM AT> REGION_TEXT` mówi linkerowi dokładnie to, a `Reset_Handler` kopiuje dane z `_sidata` do `_sdata.._edata`.

Stos główny (MSP) obsługuje przerwania i `main()` przed startem schedulera. Od etapu 2 każde zadanie dostanie własny stos (PSP), a MSP zostanie tylko dla przerwań i jądra.

### Zegar 216 MHz (`bsp/stm32f746g-disco/bsp.c`)

Po resecie rdzeń działa na wewnętrznym HSI 16 MHz. `clock_init()` przełącza go na PLL zasilane z kwarcu HSE 25 MHz:

```
HSE 25 MHz ─► /M=25 ─► 1 MHz ─► ×N=432 ─► 432 MHz ─┬─► /P=2 ─► SYSCLK 216 MHz ─┬─► AHB  216 MHz
                                                    └─► /Q=9 ─► 48 MHz (USB)    ├─► APB1  54 MHz (/4)
                                                                                └─► APB2 108 MHz (/2) ─► USART1
```

Kolejność ma znaczenie:

1. włączyć zegar PWR i ustawić skalę napięcia 1,
2. włączyć HSE i poczekać na `HSERDY`,
3. skonfigurować PLL i ją włączyć,
4. włączyć **over-drive** (`ODEN` i `ODSWEN`), bez którego rdzeń nie przekroczy 180 MHz,
5. ustawić **7 cykli oczekiwania flasha**, zanim zegar wzrośnie. Odwrotna kolejność kończy się odczytem śmieci z flasha,
6. ustawić dzielniki szyn i przełączyć `SW` na PLL.

Potem włączamy cache instrukcji i danych rdzenia M7. Przy DMA (etap 5) D-cache będzie wymagać uwagi.

### SysTick i tick systemowy

`port_init()` ustawia SysTick na `OS_TICK_HZ` = 1000 Hz i nadaje PendSV oraz SysTick najniższy priorytet. PendSV nie jest jeszcze używany, ale priorytet jest od razu poprawny. `SysTick_Handler` (port) woła `os_tick_isr()` (jądro), czyli zwykłe `tick_count++`.

### `printf` przez UART

newlib-nano kończy `printf` na funkcji `_write()` w `bsp/common/syscalls.c`. Ta zamienia `\n` na `\r\n` i wysyła znaki przez `bsp_console_putc()`. Na F746 trafiają one do USART1 (PA9/PB7), skąd przez ST-Link do `/dev/ttyACM0`. Prędkość to 115200 8N1, `BRR = 108 MHz / 115200 ≈ 938`.

`printf` z newlib-nano nie formatuje `float`. Jeśli kiedyś będzie potrzebny, dodaj `-u _printf_float` do opcji linkera (kosztuje to ok. 10 KB).

## 6. Program etapu 1

`app/main.c` to scheduler kooperacyjny z kroku 3 dokumentu 01:

| Czynność | Okres | Co robi |
|---|---|---|
| `led_job` | 500 ms | przełącza LD1, czyli dioda miga z częstotliwością 1 Hz |
| `heartbeat_job` | 1000 ms | wypisuje numer ticka |
| `echo_job` | 1 ms | odsyła opis odebranego znaku |

Między obiegami pętla wykonuje `WFI`, czyli procesor śpi do następnego ticka.

Oczekiwane wyjście w QEMU:

```
=== rtos — etap 1: bare-metal start ===
plytka: QEMU mps2-an500 (Cortex-M7), rdzen: 25 MHz, tick: 1000 Hz
[LD1 *]
[LD1 .]
tick 1000, heartbeat 1
[LD1 *]
[LD1 .]
tick 2000, heartbeat 2
```

Na płytce zamiast `[LD1 …]` miga zielona dioda, a w nagłówku pojawia się `216 MHz`.

## 7. Gdy coś pójdzie nie tak: HardFault

`port/cortex-m7/fault.c` przechwytuje HardFault i zapisuje do zmiennej `port_fault`:

- `pc`: instrukcja, która spowodowała błąd. W CLion otwórz disassembly albo sprawdź `arm-none-eabi-addr2line -e rtos.elf 0x...`,
- `lr`: skąd ta funkcja była wołana,
- `cfsr`: przyczyna. Najczęstsze bity to `UNDEFINSTR` (skok pod zły adres lub bez bitu Thumb), `INVSTATE` (bit Thumb w xPSR = 0, typowy błąd przy ręcznym budowaniu stosu zadania), `PRECISERR` z `bfar` (dostęp pod zły adres) i `STKERR`/`UNSTKERR` (zepsuty stos przy wejściu lub wyjściu z wyjątku),
- `msp_used`: 1 oznacza, że błąd wystąpił w przerwaniu lub jądrze, 0 — w zadaniu (PSP).

Gdy debugger jest podłączony, handler sam zatrzymuje się na `bkpt`.

## 8. Lista kontrolna na dzień, w którym przyjdzie płytka

1. Podłącz płytkę złączem **CN14 (ST-LINK, micro-USB)**. Powinien zaświecić się LD7 (zasilanie).
2. `st-info --probe` powinno pokazać układ `F7xx` i 1024 KB flash.
3. `ls /dev/ttyACM*` powinno pokazać `/dev/ttyACM0`. Jeśli wystąpi „permission denied”, wyloguj się i zaloguj (grupa `dialout`).
4. `cmake --build --preset f746-debug -t flash` powinno zakończyć się komunikatem `** Verified OK **`.
5. `tools/serial.sh` powinno pokazać banner z `216 MHz` i heartbeat co sekundę. Heartbeat co ~1,35 s oznacza zły zegar: kod liczy na 216 MHz, a rdzeń działa wolniej.
6. LD1 (zielona, obok przycisku) powinna migać z częstotliwością 1 Hz.
7. W CLion konfiguracja **F746: debug** z breakpointem w `main()` powinna się na nim zatrzymać.

Gdy pierwsze wgranie się nie uda: OpenOCD zgłasza `unable to connect` albo `target voltage`. Wtedy wykonaj `openocd -f bsp/stm32f746g-disco/openocd.cfg -c "init; reset halt; flash erase_sector 0 0 last; exit"` i spróbuj ponownie. W najgorszym razie przytrzymaj RESET (czarny przycisk) i puść go w chwili, gdy OpenOCD próbuje się połączyć.

## 9. QEMU a prawdziwa płytka

| | QEMU mps2-an500 | STM32F746G-DISCO |
|---|---|---|
| Rdzeń | Cortex-M7 z FPU | Cortex-M7 r0p1, FPv5-SP |
| Zegar | 25 MHz (umowny, bez dokładnych czasów) | 216 MHz |
| Cache | brak | I-cache i D-cache 4 KB + 4 KB |
| Konsola | CMSDK UART0 → terminal | USART1 → ST-Link → `/dev/ttyACM0` |
| Dioda | komunikat `[LD1 *]` w konsoli | LD1 na PI1 |
| Przydatne do | startupu, przełączania kontekstu, schedulera, synchronizacji (etapy 2–4) | wszystkiego, w tym pomiarów czasu i sterowników (etap 5+) |

QEMU dobrze sprawdza logikę jądra: PendSV, SVC, stosy, priorytety NVIC. Nie nadaje się do pomiarów czasu ani do sterowników peryferiów STM32.
