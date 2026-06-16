//
// Created by Steven Hedges on 6/2/26.
//

#ifndef NES_EMULATOR_CPP_EMULATOR_H
#define NES_EMULATOR_CPP_EMULATOR_H

#include "Bus.h"
#include "Cpu.h"

#include <vector>

namespace nes {

class Emulator {
public:
    Emulator();
    ~Emulator();
    static void Run();
    static void Step();
    // For loading test programs
    void LoadProgram(const std::vector<std::uint8_t>& program,
        const std::uint16_t start_address = Bus::RAM_START) const;

private:
    Bus bus_;
    Cpu cpu_;
};

} // nes

#endif //NES_EMULATOR_CPP_EMULATOR_H
