#include "../doctest.h"

#include <vector>

#include "../Bus.h"
#include "../Cpu.h"
#include "../Emulator.h"

TEST_CASE("STA Zero Page") {
    nes::Bus bus;
    nes::Cpu cpu(bus);
    nes::Emulator emulator(bus, cpu);
    // 0xA9 LDA Immediate, 0x42 value, 0x85 STA Zero Page, 0x10 address
    const std::vector<std::uint8_t> program{0xA9, 0x42, 0x85, 0x10};
    emulator.LoadProgram(program);
    emulator.Step();
    emulator.Step();

    CHECK(bus.ReadCpu(0x10) == 0x42);
}

TEST_CASE("STA Absolute") {
    nes::Bus bus;
    nes::Cpu cpu(bus);
    nes::Emulator emulator(bus, cpu);
    // 0xA9 LDA Immediate, 0x42 value, 0x85 STA Absolute, 0x00 low byte, 0x02 high byte
    const std::vector<std::uint8_t> program{0xA9, 0x7F, 0x8D, 0x00, 0x02};
    emulator.LoadProgram(program);
    emulator.Step();
    emulator.Step();

    CHECK(bus.ReadCpu(0x0200) == 0x7F);
}

TEST_CASE("Flags are not modified") {
    nes::Bus bus;
    nes::Cpu cpu(bus);
    nes::Emulator emulator(bus, cpu);
    // 0x00 sets Z Flag
    const std::vector<std::uint8_t> program{0xA9, 0x00, 0x85, 0x10};
    emulator.LoadProgram(program);
    emulator.Step();
    const auto status_before = cpu.GetStatusRegister();
    emulator.Step();
    const auto status_after = cpu.GetStatusRegister();

    CHECK(status_before == status_after);
}