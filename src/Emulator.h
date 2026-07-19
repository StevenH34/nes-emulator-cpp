#ifndef NES_EMULATOR_CPP_EMULATOR_H
#define NES_EMULATOR_CPP_EMULATOR_H

#include "Bus.h"
#include "cpu/Cpu.h"

#include <vector>

namespace nes {

class Emulator {
public:
    explicit Emulator(std::string path);
    ~Emulator() = default;
    // Disable copy and move operations
    Emulator(const Emulator&) = delete;
    Emulator& operator=(const Emulator&) = delete;
    Emulator(Emulator&&) = delete;
    Emulator& operator=(Emulator&&) = delete;

    static void Run();
    int Step();
    const std::vector<std::uint8_t>& RunFrame();

    // For loading test programs
    void LoadProgram(const std::vector<std::uint8_t>& program, std::uint16_t start_address = 0x0000);

    // Test helpers for inspecting internal state directly
    [[nodiscard]] Bus& GetBus() { return bus_; }
    [[nodiscard]] Cpu& GetCpu() { return cpu_; }

private:
    Cartridge cartridge_;
    Ppu ppu_;
    Bus bus_;
    Cpu cpu_;
};

} // nes

#endif //NES_EMULATOR_CPP_EMULATOR_H
