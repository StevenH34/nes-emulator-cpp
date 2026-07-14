#include "doctest.h"

#include <vector>

#include "../src/Emulator.h"

TEST_CASE("Emulator::LoadProgram copies bytes into RAM") {
    nes::Bus bus;
    nes::Cpu cpu(bus);
    nes::Emulator emulator(bus, cpu);
    const std::vector<std::uint8_t> program{0xA9, 0x42, 0xE8};
    constexpr auto start_address = 0x0200;
    emulator.LoadProgram(program, start_address);

    CHECK(bus.ReadRam(0x0200) == 0xA9);
    CHECK(bus.ReadRam(0x0201) == 0x42);
    CHECK(bus.ReadRam(0x0202) == 0xE8);
}
