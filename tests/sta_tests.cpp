#include "../doctest.h"

#include <vector>

#include "../Bus.h"
#include "../Cpu.h"
#include "../Emulator.h"

// TODO: Update tests once opcodes are implemented

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

TEST_CASE("StaZeroPageX writes the accumulator to the zero page address plus X") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x42); // value
    cpu.LdaImmediate();

    bus.WriteCpu(0x01, 0x10); // base operand
    cpu.SetXRegister(0x05);

    cpu.StaZeroPageX();

    CHECK(bus.ReadCpu(0x15) == 0x42);
}

TEST_CASE("StaAbsoluteX writes the accumulator to a 16-bit address plus X") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x42); // value
    cpu.LdaImmediate();

    bus.WriteCpu(0x01, 0x00); // low byte
    bus.WriteCpu(0x02, 0x02); // high byte, base address = 0x0200
    cpu.SetXRegister(0x10);

    cpu.StaAbsoluteX();

    CHECK(bus.ReadCpu(0x0210) == 0x42);
}

TEST_CASE("StaAbsoluteY writes the accumulator to a 16-bit address plus Y") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x42); // value
    cpu.LdaImmediate();

    bus.WriteCpu(0x01, 0x00); // low byte
    bus.WriteCpu(0x02, 0x03); // high byte, base address = 0x0300
    cpu.SetYRegister(0x20);

    cpu.StaAbsoluteY();

    CHECK(bus.ReadCpu(0x0320) == 0x42);
}

TEST_CASE("StaIndirectX adds X to the zero page base, reads a pointer, then writes the accumulator") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x42); // value
    cpu.LdaImmediate();

    bus.WriteCpu(0x01, 0x10); // base operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x78); // pointer low byte
    bus.WriteCpu(0x16, 0x06); // pointer high byte, target address = 0x0678

    cpu.StaIndirectX();

    CHECK(bus.ReadCpu(0x0678) == 0x42);
}

TEST_CASE("StaIndirectY reads a zero page pointer, adds Y, then writes the accumulator") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x42); // value
    cpu.LdaImmediate();

    bus.WriteCpu(0x01, 0x20); // zero page pointer location
    bus.WriteCpu(0x20, 0x00); // pointer low byte
    bus.WriteCpu(0x21, 0x02); // pointer high byte, base address = 0x0200
    cpu.SetYRegister(0x05);

    cpu.StaIndirectY();

    CHECK(bus.ReadCpu(0x0205) == 0x42);
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