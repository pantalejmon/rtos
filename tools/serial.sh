#!/usr/bin/env bash
# Konsola szeregowa płytki (wirtualny port COM ST-Linka). Wyjście: Ctrl+A, potem Ctrl+X.
#   tools/serial.sh [urządzenie]   (domyślnie /dev/ttyACM0)
set -euo pipefail
exec picocom -b 115200 "${1:-/dev/ttyACM0}"
