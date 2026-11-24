# QEMU, maszyna mps2-an500: płytka FPGA ARM MPS2 z rdzeniem Cortex-M7.
# Służy do pracy BEZ płytki: jądro i port Cortex-M7 są te same, zmienia się tylko BSP.
#
# Różnice względem F746: brak cache, inny UART (CMSDK), brak diody, zegar 25 MHz.
# QEMU nie emuluje dokładnych czasów — wyniki pomiarów wydajności tylko na sprzęcie.

set(RTOS_CPU_FLAGS -mcpu=cortex-m7 -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard)
add_compile_options(${RTOS_CPU_FLAGS})
add_link_options(${RTOS_CPU_FLAGS})

set(RTOS_LINKER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/mps2-an500.ld")

set(RTOS_QEMU_MACHINE mps2-an500)
