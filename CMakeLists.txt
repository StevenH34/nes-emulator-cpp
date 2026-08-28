cmake_minimum_required(VERSION 3.21)
project(nes_emulator)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(NES_ENABLE_SANITIZERS "Build with AddressSanitizer + UndefinedBehaviorSanitizer instrumentation" OFF)

add_library(nes_sanitizers INTERFACE)
if(NES_ENABLE_SANITIZERS)
    if(MSVC)
        message(WARNING "NES_ENABLE_SANITIZERS is not supported with MSVC; ignoring")
    else()
        target_compile_options(nes_sanitizers INTERFACE
                -fsanitize=address,undefined
                -fno-sanitize-recover=all
                -fno-omit-frame-pointer
                -g)
        target_link_options(nes_sanitizers INTERFACE
                -fsanitize=address,undefined)

        # implicit-conversion/integer are Clang-specific sanitizer groups; GCC
        # (used by the CI asan preset) doesn't recognize them.
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(nes_sanitizers INTERFACE -fsanitize=implicit-conversion,integer)
            target_link_options(nes_sanitizers INTERFACE -fsanitize=implicit-conversion,integer)
        endif()
    endif()
endif()

add_library(nes_warnings INTERFACE)
if(MSVC)
    target_compile_options(nes_warnings INTERFACE /W4)
else()
    target_compile_options(nes_warnings INTERFACE -Wall -Wextra -Wsign-compare -Wsign-conversion -Wconversion)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(nes_warnings INTERFACE -Wshorten-64-to-32)
    endif()
endif()

set(NES_CORE_SOURCES
        src/core/Emulator.cpp
        src/core/Emulator.h
        src/core/cpu/Cpu.cpp
        src/core/cpu/Cpu_Step.cpp
        src/core/cpu/Cpu.h
        src/core/cpu/Opcodes.h
        src/core/Bus.cpp
        src/core/Bus.h
        src/core/Cartridge.cpp
        src/core/Cartridge.h
        src/core/mappers/Mapper.h
        src/core/mappers/Mapper.cpp
        src/core/mappers/Mapper000.cpp
        src/core/mappers/Mapper000.h
        src/core/ppu/Ppu.cpp
        src/core/ppu/Ppu.h
        src/core/ppu/Ppu_Addresses.h
        src/app/Window.cpp
        src/app/Window.h
        src/app/NesApp.cpp
        src/app/NesApp.h
        src/core/Controller.cpp
        src/core/Controller.h
        src/core/apu/Apu.cpp
        src/core/apu/Apu.h
        src/core/apu/ApuConstants.h
        src/core/apu/Pulse.cpp
        src/core/apu/Pulse.h)

add_library(nes_core STATIC ${NES_CORE_SOURCES})

target_include_directories(nes_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/core)

find_package(SDL3 CONFIG REQUIRED)

target_link_libraries(nes_core PUBLIC SDL3::SDL3 nes_sanitizers)
target_link_libraries(nes_core PRIVATE nes_warnings)

add_executable(nes_emulator src/app/main.cpp)

target_link_libraries(nes_emulator PRIVATE nes_core)

if(WIN32)
    add_custom_command(TARGET nes_emulator POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_RUNTIME_DLLS:nes_emulator>"
            "$<TARGET_FILE_DIR:nes_emulator>"
            COMMAND_EXPAND_LISTS)
endif()

enable_testing()

add_executable(nes_emulator_tests
        tests/test_main.cpp
        tests/TestRom.h
        tests/TestRom.cpp
        tests/TestBus.h
        tests/TestBus.cpp
        tests/Bus_tests.cpp
        tests/Controller_tests.cpp
        tests/Pulse_tests.cpp
        tests/Emulator_tests.cpp
        tests/Cartridge_tests.cpp
        tests/test_nestest.cpp
        tests/cpu/Cpu_tests.cpp
        tests/cpu/addressing_mode_tests.cpp
        tests/cpu/opcodes/sta_tests.cpp
        tests/cpu/opcodes/lda_tests.cpp
        tests/cpu/opcodes/ldx_tests.cpp
        tests/cpu/opcodes/ldy_tests.cpp
        tests/cpu/opcodes/stx_tests.cpp
        tests/cpu/opcodes/sty_tests.cpp
        tests/cpu/opcodes/branch_jump_tests.cpp
        tests/cpu/opcodes/stack_tests.cpp
        tests/cpu/opcodes/compare_tests.cpp
        tests/cpu/opcodes/asl_tests.cpp
        tests/cpu/opcodes/lsr_tests.cpp
        tests/cpu/opcodes/rol_tests.cpp
        tests/cpu/opcodes/ror_tests.cpp
        tests/cpu/opcodes/adc_tests.cpp
        tests/cpu/opcodes/and_tests.cpp
        tests/cpu/opcodes/ora_tests.cpp
        tests/cpu/opcodes/eor_tests.cpp
        tests/cpu/opcodes/register_increment_tests.cpp
        tests/cpu/opcodes/register_transfer_tests.cpp
        tests/cpu/opcodes/flag_instruction_tests.cpp
        tests/cpu/opcodes/sbc_tests.cpp
        tests/cpu/opcodes/inc_tests.cpp
        tests/cpu/opcodes/dec_tests.cpp
        tests/cpu/opcodes/bit_tests.cpp
        tests/cpu/opcodes/nop_tests.cpp
        tests/ppu/Ppu_tests.cpp
        tests/mappers/Mapper000_tests.cpp
)

target_link_libraries(nes_emulator_tests PRIVATE nes_core nes_warnings)

target_include_directories(nes_emulator_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/external
        ${CMAKE_CURRENT_SOURCE_DIR}/tests)

target_compile_definitions(nes_emulator_tests PRIVATE
        TEST_ROMS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/roms/")

if(WIN32)
    add_custom_command(TARGET nes_emulator_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_RUNTIME_DLLS:nes_emulator_tests>"
            "$<TARGET_FILE_DIR:nes_emulator_tests>"
            COMMAND_EXPAND_LISTS)
endif()

add_test(NAME nes_emulator_tests COMMAND nes_emulator_tests)

# Build and run the tests in one step: cmake --build cmake-build-debug --target run_tests
add_custom_target(run_tests
        COMMAND nes_emulator_tests
        DEPENDS nes_emulator_tests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running nes_emulator_tests")

# Static analysis via .clang-tidy: cmake --build cmake-build-debug --target clang-tidy
find_program(CLANG_TIDY_EXE
        NAMES clang-tidy
        HINTS /opt/homebrew/opt/llvm/bin /usr/local/opt/llvm/bin)

if(CLANG_TIDY_EXE)
    set(NES_TIDY_SOURCES ${NES_CORE_SOURCES})
    list(FILTER NES_TIDY_SOURCES INCLUDE REGEX "\\.cpp$")

    set(NES_TIDY_EXTRA_ARGS "")
    if(APPLE)
        # Homebrew's clang-tidy doesn't discover the Xcode SDK on its own.
        execute_process(COMMAND xcrun --show-sdk-path
                OUTPUT_VARIABLE NES_APPLE_SDK_PATH
                OUTPUT_STRIP_TRAILING_WHITESPACE)
        list(APPEND NES_TIDY_EXTRA_ARGS "--extra-arg=-isysroot${NES_APPLE_SDK_PATH}")
    endif()

    add_custom_target(clang-tidy
            COMMAND ${CLANG_TIDY_EXE} -p ${CMAKE_BINARY_DIR} ${NES_TIDY_EXTRA_ARGS} ${NES_TIDY_SOURCES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Running clang-tidy over nes_core sources")
else()
    message(STATUS "clang-tidy not found; 'clang-tidy' target will be unavailable")
endif()