#include "Emulator.h"

#include <iostream>

namespace nes {

Emulator::Emulator(std::string path)
    : cartridge_(std::move(path)), ppu_(cartridge_), bus_(cartridge_, ppu_), cpu_(bus_) {
    ppu_.SetNmiCallback([this] { cpu_.Nmi(); });
    cpu_.Reset();
}

void Emulator::Run() {
    std::puts("Hello, World");
}

int Emulator::Step() {
    // Ticks the CPU forward
    auto cycles = cpu_.Step();
    for (size_t i = 0; i < cycles * 3; ++i) {
        ppu_.Step();
    }
    return cycles;
}

const std::vector<std::uint8_t>& Emulator::RunFrame() {
    if (ppu_.IsFrameComplete()) {
        ppu_.ClearFrameComplete();
    }
    while (!ppu_.IsFrameComplete()) {
        Step();
    }
    return ppu_.GetFrameBuffer();
}

void Emulator::LoadProgram(const std::vector<std::uint8_t> &program, const std::uint16_t start_address) {
    auto address = start_address;

    for (const auto byte : program) {
        bus_.WriteCpu(address, byte);
        ++address;
    }
}

} // nes