//
// Created by Steven Hedges on 6/2/26.
//

#include "Emulator.h"

#include <iostream>

namespace nes {

Emulator::Emulator() : bus_(), cpu_(bus_) {}
Emulator::~Emulator() = default;
void Emulator::Run() {
    std::puts("Hello, World");
}

void Emulator::Step() {
    // Ticks the CPU forward
    auto cycles = Cpu::Step();
}

void Emulator::LoadProgram(const std::vector<std::uint8_t> &program, const std::uint16_t start_address) const {
    auto address = start_address;

    for (const auto byte : program) {
        bus_.WriteCpu(address, byte);
        ++address;
    }
}
} // nes