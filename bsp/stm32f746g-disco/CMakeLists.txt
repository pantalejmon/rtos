set(THIRD_PARTY ${PROJECT_SOURCE_DIR}/third_party)

add_library(cmsis_device INTERFACE)
target_include_directories(cmsis_device INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${THIRD_PARTY}/cmsis/Core/Include
    ${THIRD_PARTY}/cmsis-device-f7/Include
)
target_compile_definitions(cmsis_device INTERFACE STM32F746xx)

# OBJECT, nie STATIC: handlery przerwań muszą trafić do obrazu, a słabe (weak)
# symbole ze startupu nie wciągnęłyby ich z archiwum .a.
add_library(bsp OBJECT bsp.c)
target_link_libraries(bsp PUBLIC cmsis_device)
