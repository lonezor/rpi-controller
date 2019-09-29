cmake_minimum_required(VERSION 3.10.2)

project(rpi-controller)

set(CMAKE_BUILD_TYPE Debug)

include_directories(src)

add_library(common
    STATIC
    src/adt.c
    src/common.c
    src/gpio.c
)

add_executable(pwr_ctrl
    src/pwr_button.c
    src/pwr_ctrl.c
    src/piezo.c
    src/temperature.c
)

add_executable(relay_ctrl
    src/relay_ctrl.c
)

set_property(TARGET relay_ctrl PROPERTY C_STANDARD 99)
set_property(TARGET pwr_ctrl PROPERTY C_STANDARD 99)


target_link_libraries(relay_ctrl
    PRIVATE
    common
)

target_link_libraries(pwr_ctrl
    PRIVATE
    common
    pthread
)

install(TARGETS relay_ctrl DESTINATION /sbin)
install(TARGETS pwr_ctrl DESTINATION /sbin)




