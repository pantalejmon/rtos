set(THIRD_PARTY ${PROJECT_SOURCE_DIR}/third_party)

add_library(cmsis_device INTERFACE)
target_include_directories(cmsis_device INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${THIRD_PARTY}/cmsis/Core/Include
)

add_library(bsp OBJECT bsp.c)
target_link_libraries(bsp PUBLIC cmsis_device)
