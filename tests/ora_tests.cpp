#include "../doctest.h"

#include "../src/Bus.h"
#include "../src/Cpu.h"

TEST_CASE("OraImmediate ORs the value following the opcode with the accumulator") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x00);
    bus.WriteCpu(0x00, 0x00); // value, result = 0x00

    cpu.OraImmediate();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc");
}

TEST_CASE("OraImmediate sets the Negative flag when the result has bit 7 set") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x0F);
    bus.WriteCpu(0x00, 0x80); // value, result = 0x8F

    cpu.OraImmediate();

    CHECK(cpu.GetAccumulator() == 0x8F);
    CHECK(cpu.StatusString() == "NvUbdIzc");
}

TEST_CASE("OraZeroPage ORs the value at the zero page address with the accumulator") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x3C);
    bus.WriteCpu(0x00, 0x10); // zero page address
    bus.WriteCpu(0x10, 0x03); // value, result = 0x3F

    cpu.OraZeroPage();

    CHECK(cpu.GetAccumulator() == 0x3F);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("OraZeroPageX ORs the value at the zero page address plus X with the accumulator") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x00);
    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x00); // value, result = 0x00

    cpu.OraZeroPageX();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc");
}

TEST_CASE("OraAbsolute ORs the value at a 16-bit address with the accumulator") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x00);
    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, address = 0x0200
    bus.WriteCpu(0x0200, 0x80); // value, result = 0x80

    cpu.OraAbsolute();

    CHECK(cpu.GetAccumulator() == 0x80);
    CHECK(cpu.StatusString() == "NvUbdIzc");
}

TEST_CASE("OraAbsoluteX ORs the value at a 16-bit address plus X with the accumulator") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x0F);
    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, base address = 0x0200
    cpu.SetXRegister(0x10);
    bus.WriteCpu(0x0210, 0x10); // value, result = 0x1F

    cpu.OraAbsoluteX();

    CHECK(cpu.GetAccumulator() == 0x1F);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("OraAbsoluteY ORs the value at a 16-bit address plus Y with the accumulator") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x00);
    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x03); // high byte, base address = 0x0300
    cpu.SetYRegister(0x20);
    bus.WriteCpu(0x0320, 0x00); // value, result = 0x00

    cpu.OraAbsoluteY();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc");
}

TEST_CASE("OraIndirectX adds X to the zero page base, reads a pointer, then ORs the value") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x70);
    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x78); // pointer low byte
    bus.WriteCpu(0x16, 0x06); // pointer high byte, target address = 0x0678
    bus.WriteCpu(0x0678, 0x80); // value, result = 0xF0

    cpu.OraIndirectX();

    CHECK(cpu.GetAccumulator() == 0xF0);
    CHECK(cpu.StatusString() == "NvUbdIzc");
}

TEST_CASE("OraIndirectY reads a zero page pointer, adds Y, then ORs the value") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x00);
    bus.WriteCpu(0x00, 0x20); // zero page pointer location
    bus.WriteCpu(0x20, 0x00); // pointer low byte
    bus.WriteCpu(0x21, 0x02); // pointer high byte, base address = 0x0200
    cpu.SetYRegister(0x05);
    bus.WriteCpu(0x0205, 0x00); // value, result = 0x00

    cpu.OraIndirectY();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc");
}