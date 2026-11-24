cmake_minimum_required(VERSION 3.25)

# Płytka docelowa = nazwa katalogu w bsp/. Najwygodniej wybierać ją presetem
# (CMakePresets.json), CLion pokaże presety jako profile CMake.
set(RTOS_BOARD "stm32f746g-disco" CACHE STRING "Płytka docelowa (katalog w bsp/)")
set_property(CACHE RTOS_BOARD PROPERTY STRINGS stm32f746g-disco qemu-mps2-an500)

if(NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/cmake/arm-none-eabi.cmake")
endif()

project(rtos LANGUAGES C ASM)

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/bsp/${RTOS_BOARD}/board.cmake")
    message(FATAL_ERROR "Nieznana płytka RTOS_BOARD='${RTOS_BOARD}' (brak bsp/${RTOS_BOARD}/board.cmake)")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)            # gnu11: potrzebne m.in. dla __asm__ i atrybutów
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_FLAGS_DEBUG          "-Og -g3")
set(CMAKE_C_FLAGS_RELEASE        "-O2 -g -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g3 -DNDEBUG")
set(CMAKE_ASM_FLAGS_DEBUG        "-g3")
set(CMAKE_ASM_FLAGS_RELEASE      "-g")

# board.cmake ustawia: flagi CPU (add_compile_options/add_link_options),
# RTOS_LINKER_SCRIPT oraz opcjonalnie sposób uruchamiania (flash / qemu).
include(bsp/${RTOS_BOARD}/board.cmake)

add_compile_options(
    -ffunction-sections -fdata-sections -fno-common
    -Wall -Wextra -Wshadow -Wundef -Wdouble-promotion -Wstrict-prototypes
)

add_subdirectory(bsp)
add_subdirectory(port/cortex-m7)
add_subdirectory(kernel)
add_subdirectory(app)
