#include "../doctest.h"

#include <vector>

#include "../Bus.h"
#include "../Emulator.h"

TEST_CASE("Emulator::LoadProgram copies bytes into RAM") {
    nes::Emulator emulator;
    const std::vector<std::uint8_t> program{0xA9, 0x42, 0xE8};
    constexpr auto start_address = 0x0200;
    emulator.LoadProgram(program, start_address);

    CHECK(nes::Bus::ReadCpu(0x0200) == 0xA9);
    CHECK(nes::Bus::ReadCpu(0x0201) == 0x42);
    CHECK(nes::Bus::ReadCpu(0x0202) == 0xE8);
}

TEST_CASE("INX increment test") {
    nes::Emulator emulator;
    const std::vector<std::uint8_t> program{0xE8};
}

