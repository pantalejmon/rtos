add_executable(rtos main.c)
set_target_properties(rtos PROPERTIES SUFFIX ".elf")

# Każda biblioteka OBJECT musi być wymieniona wprost: zależność przechodnia
# (kernel → port) przekazuje tylko nagłówki, a nie pliki .o.
target_link_libraries(rtos PRIVATE kernel port bsp)

target_link_options(rtos PRIVATE
    -T${RTOS_LINKER_SCRIPT}
    -L${PROJECT_SOURCE_DIR}/bsp/common          # INCLUDE sections.ld
    --specs=nano.specs                          # newlib-nano: mniejszy printf/malloc
    -Wl,--gc-sections                           # wyrzuć nieużywane funkcje (z -ffunction-sections)
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/rtos.map
    -Wl,--print-memory-usage
    -Wl,--no-warn-rwx-segments
)
set_property(TARGET rtos APPEND PROPERTY LINK_DEPENDS
    ${RTOS_LINKER_SCRIPT} ${PROJECT_SOURCE_DIR}/bsp/common/sections.ld)

# Po zbudowaniu: .bin / .hex (dla programatorów) i rozmiar sekcji.
add_custom_command(TARGET rtos POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:rtos> $<TARGET_FILE_DIR:rtos>/rtos.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:rtos> $<TARGET_FILE_DIR:rtos>/rtos.hex
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:rtos>
    VERBATIM
)

# --- Uruchamianie ---------------------------------------------------------
# Cele pomocnicze: w CLion wybierz je z listy konfiguracji i naciśnij Build (Ctrl+F9).

if(DEFINED RTOS_OPENOCD_CFG)
    find_program(OPENOCD openocd)
    add_custom_target(flash
        COMMAND ${OPENOCD} -f ${RTOS_OPENOCD_CFG}
                -c "program $<TARGET_FILE:rtos> verify reset exit"
        DEPENDS rtos
        USES_TERMINAL
        COMMENT "Wgrywanie rtos.elf przez ST-Link"
    )
endif()

if(DEFINED RTOS_QEMU_MACHINE)
    find_program(QEMU_SYSTEM_ARM qemu-system-arm)
    add_custom_target(qemu
        COMMAND ${QEMU_SYSTEM_ARM} -M ${RTOS_QEMU_MACHINE} -nographic -kernel $<TARGET_FILE:rtos>
        DEPENDS rtos
        USES_TERMINAL
        COMMENT "QEMU ${RTOS_QEMU_MACHINE} — wyjście: Ctrl+A, potem X"
    )
endif()
