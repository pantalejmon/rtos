# STM32F746NGH6: Cortex-M7 r0p1, FPU pojedynczej precyzji (FPv5-SP), 1 MB flash, 320 KB RAM.

set(RTOS_CPU_FLAGS -mcpu=cortex-m7 -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard)
add_compile_options(${RTOS_CPU_FLAGS})
add_link_options(${RTOS_CPU_FLAGS})

set(RTOS_LINKER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/STM32F746NGHx.ld")

# Wgrywanie przez ST-Link wbudowany w płytkę (cel `flash`, patrz app/CMakeLists.txt).
set(RTOS_OPENOCD_CFG "${CMAKE_CURRENT_LIST_DIR}/openocd.cfg")
