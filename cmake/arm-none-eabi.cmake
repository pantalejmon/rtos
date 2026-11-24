# Toolchain file dla arm-none-eabi-gcc (bare-metal, Cortex-M).
#
# Domyślnie kompilator jest szukany w PATH (pakiet gcc-arm-none-eabi z Ubuntu).
# Żeby użyć innego toolchaina (np. STM32CubeCLT albo ARM GNU Toolchain),
# ustaw ARM_TOOLCHAIN_DIR na katalog z binarkami:
#   cmake --preset f746-debug -DARM_TOOLCHAIN_DIR=/opt/st/stm32cubeclt/GNU-tools-for-STM32/bin

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(ARM_TOOLCHAIN_DIR "" CACHE PATH "Katalog z binarkami arm-none-eabi-* (puste = PATH)")
if(ARM_TOOLCHAIN_DIR)
    set(_tc "${ARM_TOOLCHAIN_DIR}/arm-none-eabi-")
else()
    set(_tc "arm-none-eabi-")
endif()

set(CMAKE_C_COMPILER   "${_tc}gcc")
set(CMAKE_ASM_COMPILER "${_tc}gcc")
set(CMAKE_CXX_COMPILER "${_tc}g++")
set(CMAKE_OBJCOPY      "${_tc}objcopy" CACHE FILEPATH "objcopy")
set(CMAKE_SIZE         "${_tc}size"    CACHE FILEPATH "size")

# Testowa kompilacja CMake nie może linkować programu (brak startupu i linker scriptu).
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
