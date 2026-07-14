//
// Created by Steven Hedges on 6/2/26.
//

#include "Emulator.h"

#include <iostream>

namespace nes {

Emulator::Emulator(std::string path)
    : cartridge_(std::move(path)), bus_(cartridge_), cpu_(bus_) {
    cpu_.Reset();
}

void Emulator::Run() {
    std::puts("Hello, World");
}

void Emulator::Step() {
    // Ticks the CPU forward
    auto cycles = cpu_.Step();
}

void Emulator::LoadProgram(const std::vector<std::uint8_t> &program, const std::uint16_t start_address) {
    auto address = start_address;

    for (const auto byte : program) {
        bus_.WriteCpu(address, byte);
        ++address;
    }
}

} // nes