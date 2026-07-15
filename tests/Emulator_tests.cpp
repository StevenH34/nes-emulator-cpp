#include "doctest.h"

#include <vector>

#include "../src/Emulator.h"
#include "TestRom.h"

TEST_CASE("Emulator::LoadProgram copies bytes into RAM") {
    const nes_test::TempRomFile rom(nes_test::MakeMinimalRom(0x0200));
    nes::Emulator emulator(rom.path());
    const std::vector<std::uint8_t> program{0xA9, 0x42, 0xE8};
    constexpr auto start_address = 0x0200;
    emulator.LoadProgram(program, start_address);

    CHECK(emulator.GetBus().ReadRam(0x0200) == 0xA9);
    CHECK(emulator.GetBus().ReadRam(0x0201) == 0x42);
    CHECK(emulator.GetBus().ReadRam(0x0202) == 0xE8);
}

TEST_CASE("Emulator constructor resets the CPU using the cartridge's reset vector") {
    const nes_test::TempRomFile rom(nes_test::MakeMinimalRom(0x0200));
    nes::Emulator emulator(rom.path());

    CHECK(emulator.GetCpu().GetProgramCounter() == 0x0200);
}

TEST_CASE("Emulator::Step executes the loaded program starting at the reset vector") {
    const nes_test::TempRomFile rom(nes_test::MakeMinimalRom(0x0200));
    nes::Emulator emulator(rom.path());
    const std::vector<std::uint8_t> program{0xA9, 0x42, 0xE8}; // LDA #0x42, INX
    emulator.LoadProgram(program, 0x0200);

    emulator.Step(); // LDA #0x42
    CHECK(emulator.GetCpu().GetAccumulator() == 0x42);

    emulator.Step(); // INX
    CHECK(emulator.GetCpu().GetXRegister() == 0x01);
}