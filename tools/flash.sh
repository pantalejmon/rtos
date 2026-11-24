#!/usr/bin/env bash
# Wgrywa firmware na STM32F746G-DISCO przez wbudowany ST-Link.
#   tools/flash.sh [plik.elf]     (domyślnie build/f746-debug/app/rtos.elf)
set -euo pipefail
cd "$(dirname "$0")/.."

ELF="${1:-build/f746-debug/app/rtos.elf}"
[[ -f "$ELF" ]] || { echo "Brak $ELF — najpierw: cmake --build --preset f746-debug" >&2; exit 1; }

exec openocd -f bsp/stm32f746g-disco/openocd.cfg -c "program $ELF verify reset exit"
