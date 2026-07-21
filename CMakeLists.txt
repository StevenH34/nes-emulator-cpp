cmake_minimum_required(VERSION 4.2)
project(nes_emulator_cpp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(nes_core STATIC
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
        "src/core/ppu/Ppu_Addresses.h"
        src/app/Window.cpp
        src/app/Window.h
        src/app/NesApp.cpp
        src/app/NesApp.h)

target_include_directories(nes_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src/core)

find_package(SDL3 CONFIG REQUIRED)

target_link_libraries(nes_core PUBLIC SDL3::SDL3)

add_executable(nes_emulator_cpp src/app/main.cpp)

target_link_libraries(nes_emulator_cpp PRIVATE nes_core)

add_custom_command(TARGET nes_emulator_cpp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_RUNTIME_DLLS:nes_emulator_cpp>"
        "$<TARGET_FILE_DIR:nes_emulator_cpp>"
        COMMAND_EXPAND_LISTS)

enable_testing()

add_executable(nes_emulator_tests
        tests/test_main.cpp
        tests/TestRom.h
        tests/TestRom.cpp
        tests/TestBus.h
        tests/TestBus.cpp
        tests/Bus_tests.cpp
        tests/Cpu_tests.cpp
        tests/Emulator_tests.cpp
        tests/Cartridge_tests.cpp
        tests/Mapper000_tests.cpp
        tests/Ppu_tests.cpp
        tests/sta_tests.cpp
        tests/addressing_mode_tests.cpp
        tests/lda_tests.cpp
        tests/ldx_tests.cpp
        tests/ldy_tests.cpp
        tests/stx_tests.cpp
        tests/sty_tests.cpp
        tests/branch_jump_tests.cpp
        tests/stack_tests.cpp
        tests/compare_tests.cpp
        tests/asl_tests.cpp
        tests/lsr_tests.cpp
        tests/rol_tests.cpp
        tests/ror_tests.cpp
        tests/adc_tests.cpp
        tests/and_tests.cpp
        tests/ora_tests.cpp
        tests/eor_tests.cpp
        tests/register_increment_tests.cpp
        tests/register_transfer_tests.cpp
        tests/flag_instruction_tests.cpp
        tests/sbc_tests.cpp
        tests/inc_tests.cpp
        tests/dec_tests.cpp
        tests/bit_tests.cpp
        tests/nop_tests.cpp
        tests/test_nestest.cpp
)

target_link_libraries(nes_emulator_tests PRIVATE nes_core)

target_include_directories(nes_emulator_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external)

target_compile_definitions(nes_emulator_tests PRIVATE
        TEST_ROMS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/roms/")

add_test(NAME nes_emulator_tests COMMAND nes_emulator_tests)

# Build and run the tests in one step: cmake --build cmake-build-debug --target run_tests
add_custom_target(run_tests
        COMMAND nes_emulator_tests
        DEPENDS nes_emulator_tests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running nes_emulator_tests")