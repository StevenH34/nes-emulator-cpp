cmake_minimum_required(VERSION 4.2)
project(nes_emulator_cpp)

set(CMAKE_CXX_STANDARD 23)

add_executable(nes_emulator_cpp
        main.cpp
        Emulator.cpp
        Emulator.h
        Cpu.cpp
        Cpu.h
        Bus.cpp
        Bus.h)

target_include_directories(nes_emulator_cpp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

enable_testing()

add_executable(nes_emulator_tests
        tests/test_main.cpp
        tests/Bus_tests.cpp
        tests/Cpu_tests.cpp
        tests/Emulator_tests.cpp
        Bus.cpp
        Cpu.cpp
        Emulator.cpp
        tests/sta_tests.cpp
        tests/AddressingMode_tests.cpp)

target_include_directories(nes_emulator_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_test(NAME nes_emulator_tests COMMAND nes_emulator_tests)

# Build and run the tests in one step: cmake --build cmake-build-debug --target run_tests
add_custom_target(run_tests
        COMMAND nes_emulator_tests
        DEPENDS nes_emulator_tests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running nes_emulator_tests")
