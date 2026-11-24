# Port Cortex-M7: wszystko, co zależy od rdzenia, ale nie od płytki.
# Etap 2 dołoży tu context_switch.S (PendSV, SVC).
add_library(port OBJECT
    port.c
    fault.c
)
target_include_directories(port
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${PROJECT_SOURCE_DIR}/kernel/include
)
target_link_libraries(port PUBLIC cmsis_device)
