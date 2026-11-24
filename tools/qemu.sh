#!/usr/bin/env bash
# Uruchamia firmware w QEMU (maszyna mps2-an500, Cortex-M7).
#
#   tools/qemu.sh            — zwykłe uruchomienie, wyjście: Ctrl+A, potem X
#   tools/qemu.sh --debug    — czeka na GDB na porcie 1234 (CLion: konfiguracja „QEMU: debug”)
#
# Domyślny plik: build/qemu-debug/app/rtos.elf (preset qemu-debug). Inny: RTOS_ELF=ścieżka.
set -euo pipefail
cd "$(dirname "$0")/.."

ELF="${RTOS_ELF:-build/qemu-debug/app/rtos.elf}"
if [[ ! -f "$ELF" ]]; then
    echo "Brak $ELF — najpierw: cmake --preset qemu-debug && cmake --build --preset qemu-debug" >&2
    exit 1
fi

ARGS=(-M mps2-an500 -nographic -kernel "$ELF")
if [[ "${1:-}" == "--debug" ]]; then
    ARGS+=(-S -gdb tcp::1234)
    echo ">> QEMU zatrzymany na resecie, czeka na GDB: localhost:1234"
fi
exec qemu-system-arm "${ARGS[@]}"
