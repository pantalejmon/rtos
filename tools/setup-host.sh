#!/usr/bin/env bash
# Przygotowanie komputera (Ubuntu 26.04) do pracy nad projektem.
# Uruchom raz:  sudo ./tools/setup-host.sh
set -euo pipefail

if [[ $EUID -ne 0 ]]; then
    echo "Uruchom przez sudo: sudo $0" >&2
    exit 1
fi

PKGS=(
    gcc-arm-none-eabi          # kompilator C dla Cortex-M
    libnewlib-arm-none-eabi    # biblioteka C (newlib + newlib-nano)
    binutils-arm-none-eabi     # as, ld, objcopy, size, objdump
    gdb-multiarch              # debugger rozumiejący ARM (CLion może też użyć własnego)
    openocd                    # serwer GDB i programator przez ST-Link
    stlink-tools               # st-info, st-flash — szybki test, czy płytka odpowiada
    qemu-system-arm            # emulator: maszyna mps2-an500 z Cortex-M7
    picocom                    # terminal szeregowy do /dev/ttyACM0
    cmake ninja-build
)

apt-get update
apt-get install -y "${PKGS[@]}"

# Reguły udev dla ST-Linka (dostęp bez roota) instalują pakiety openocd i stlink-tools.
udevadm control --reload-rules
udevadm trigger

# /dev/ttyACM0 (wirtualny port COM ST-Linka) należy do grupy dialout.
USER_NAME="${SUDO_USER:-}"
if [[ -n "$USER_NAME" ]] && ! id -nG "$USER_NAME" | grep -qw dialout; then
    usermod -aG dialout "$USER_NAME"
    echo ">> Dodano $USER_NAME do grupy dialout — wyloguj się i zaloguj ponownie."
fi

echo
echo "== Zainstalowane wersje =="
arm-none-eabi-gcc --version | head -1
gdb-multiarch --version | head -1
openocd --version 2>&1 | head -1
qemu-system-arm --version | head -1
echo
echo "Gotowe. Test bez płytki:  cmake --preset qemu-debug && cmake --build --preset qemu-debug --target qemu"
