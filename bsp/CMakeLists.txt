# Wspólna część BSP: interfejs bsp.h (taki sam dla każdej płytki), startup i syscalls.
# Właściwa implementacja jest w bsp/${RTOS_BOARD}/, która tworzy:
#   - cmsis_device  (INTERFACE) — nagłówki CMSIS, cmsis_device.h i irq_*.inc danej płytki
#   - bsp           (OBJECT)    — zegary, LED, UART

add_subdirectory(${RTOS_BOARD})

target_include_directories(bsp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_sources(bsp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/common/startup.c
    ${CMAKE_CURRENT_SOURCE_DIR}/common/syscalls.c
)
