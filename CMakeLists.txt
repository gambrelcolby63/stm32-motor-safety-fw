cmake_minimum_required(VERSION 3.20)

if(CMAKE_TOOLCHAIN_FILE)
    # The toolchain file sets the processor before the project() call.
endif()

project(pan_tilt_gimbal C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
    set(MC_FIRMWARE TRUE)
else()
    set(MC_FIRMWARE FALSE)
endif()

set(MC_WARN_BOTH
    -Wall -Wextra -Wpedantic -Werror -Wshadow -Wundef
    -Wconversion -Wdouble-promotion -Wformat=2
)
set(MC_WARN_C -Wstrict-prototypes)

set(MC_CORE_SOURCES
    src/control/names.c
    src/control/pi.c
    src/control/trajectory.c
    src/control/axis.c
    src/control/analog.c
    src/control/homing.c
    src/cpp/keepout.cpp
    src/cpp/command_parser.cpp
    src/safety/state_machine.c
    src/platform/gpio.c
    src/platform/pwm.c
    src/platform/encoder.c
    src/platform/adc.c
    src/platform/uart.c
    src/platform/systick.c
    src/platform/iwdg.c
    src/platform/board.c
    src/app/shell.c
    src/app/app.c
)

if(MC_FIRMWARE)
    add_compile_options(
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard
        -ffunction-sections
        -fdata-sections
        -fno-common
        $<$<COMPILE_LANGUAGE:C>:-ffreestanding>
    )
    # One flag per genex. A semicolon list inside a single genex is split
    # before Ninja quotes it, so C files see a stray "-fno-threadsafe-statics>".
    add_compile_options(
        $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>
    )

    add_executable(gimbal
        ${MC_CORE_SOURCES}
        src/platform/system_stm32f4xx.c
        src/platform/syscalls.c
        src/app/main.c
        startup/startup_stm32f407xx.s
    )
    target_include_directories(gimbal PRIVATE include)
    target_include_directories(gimbal SYSTEM PRIVATE third_party/CMSIS/Core/Include)
    target_compile_definitions(gimbal PRIVATE STM32F407xx)
    target_compile_options(gimbal PRIVATE
        ${MC_WARN_BOTH}
        $<$<COMPILE_LANGUAGE:C>:${MC_WARN_C}>
    )
    target_link_options(gimbal PRIVATE
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard
        -Wl,--gc-sections
        -specs=nano.specs
        -specs=nosys.specs
        -nostartfiles
        -T${CMAKE_SOURCE_DIR}/linker/stm32f407vg.ld
        -Wl,-Map=gimbal.map
    )
    target_link_libraries(gimbal PRIVATE m)
    set_target_properties(gimbal PROPERTIES OUTPUT_NAME gimbal SUFFIX ".elf")

    add_custom_command(TARGET gimbal POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:gimbal> gimbal.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:gimbal> gimbal.bin
        COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:gimbal>
        VERBATIM
    )
else()
    add_library(gimbal_core ${MC_CORE_SOURCES} src/platform/host_hw.c src/safety/trace.c)
    target_include_directories(gimbal_core PUBLIC include)
    target_compile_definitions(gimbal_core PUBLIC MC_HOST_BUILD)
    target_compile_options(gimbal_core PRIVATE
        ${MC_WARN_BOTH}
        $<$<COMPILE_LANGUAGE:C>:${MC_WARN_C}>
    )

    include(FetchContent)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    add_executable(gimbal_tests
        tests/host/test_main.cpp
        tests/host/test_pi.cpp
        tests/host/test_trajectory.cpp
        tests/host/test_keepout.cpp
        tests/host/test_homing.cpp
        tests/host/test_axis.cpp
        tests/host/test_safety.cpp
        tests/host/test_analog.cpp
        tests/host/test_drivers.cpp
        tests/host/test_parser.cpp
        tests/host/test_app.cpp
    )
    target_link_libraries(gimbal_core PUBLIC m)
    target_compile_definitions(gimbal_tests PRIVATE MC_HOST_BUILD)
    target_link_libraries(gimbal_tests PRIVATE gimbal_core GTest::gtest)
    target_compile_options(gimbal_tests PRIVATE -Wall -Wextra -Werror -Wshadow)
    add_test(NAME host COMMAND gimbal_tests)
endif()
