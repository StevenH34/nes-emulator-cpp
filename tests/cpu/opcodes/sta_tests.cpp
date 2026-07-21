#include "doctest.h"

#include "../../../src/core/Bus.h"
#include "../../../src/core/cpu/Cpu.h"
#include "TestBus.h"

TEST_CASE("StaZeroPage writes the accumulator to the zero page address") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x42); // value
    cpu.LdaImmediate();

    bus.WriteCpu(0x01, 0x10); // zero page address

    cpu.StaZeroPage();

    CHECK(cpu.GetProgramCounter() == 0x0002); // one operand byte consumed
    CHECK(bus.ReadCpu(0x10) == 0x42);
}

TEST_CASE("StaAbsolute writes the accumulator to a 16-bit address") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x7F); // value
    cpu.LdaImmediate();

    bus.WriteCpu(0x01, 0x00); // low byte
    bus.WriteCpu(0x02, 0x02); // high byte, address = 0x0200

    cpu.StaAbsolute();

    CHECK(cpu.GetProgramCounter() == 0x0003); // two operand bytes consumed
    CHECK(bus.ReadCpu(0x0200) == 0x7F);
}

TEST_CASE("StaZeroPageX writes the accumulator to the zero page address plus X") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x42); // value
    cpu.LdaImmediate();

    bus.WriteCpu(0x01, 0x10); // base operand
    cpu.SetXRegister(0x05);

    cpu.StaZeroPageX();

    CHECK(bus.ReadCpu(0x15) == 0x42);
}

TEST_CASE("StaAbsoluteX writes the accumulator to a 16-bit address plus X") {
    nes_test::TestBus bus;
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
    nes_test::TestBus bus;
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
    nes_test::TestBus bus;
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
    nes_test::TestBus bus;
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

TEST_CASE("StaZeroPage does not modify any flags") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // value, LdaImmediate sets the Zero flag
    cpu.LdaImmediate();
    const auto status_before = cpu.GetStatusRegister();

    bus.WriteCpu(0x01, 0x10); // zero page address
    cpu.StaZeroPage();

    CHECK(cpu.GetStatusRegister() == status_before);
}