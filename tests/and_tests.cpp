#include "doctest.h"

#include "../src/core/Bus.h"
#include "TestBus.h"
#include "../src/core/cpu/Cpu.h"

TEST_CASE("AndImmediate ANDs the value following the opcode with the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xF0);
    bus.WriteCpu(0x00, 0x0F); // value, result = 0x00

    cpu.AndImmediate();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc");
}

TEST_CASE("AndImmediate sets the Negative flag when the high bit is retained") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xFF);
    bus.WriteCpu(0x00, 0x80); // value, result = 0x80

    cpu.AndImmediate();

    CHECK(cpu.GetAccumulator() == 0x80);
    CHECK(cpu.StatusString() == "NvUbdIzc");
}

TEST_CASE("AndZeroPage ANDs the value at the zero page address with the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x3C);
    bus.WriteCpu(0x00, 0x10); // zero page address
    bus.WriteCpu(0x10, 0x0F); // value, result = 0x0C

    cpu.AndZeroPage();

    CHECK(cpu.GetAccumulator() == 0x0C);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("AndZeroPageX ANDs the value at the zero page address plus X with the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xFF);
    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x00); // value, result = 0x00

    cpu.AndZeroPageX();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc");
}

TEST_CASE("AndAbsolute ANDs the value at a 16-bit address with the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xFF);
    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, address = 0x0200
    bus.WriteCpu(0x0200, 0x80); // value, result = 0x80

    cpu.AndAbsolute();

    CHECK(cpu.GetAccumulator() == 0x80);
    CHECK(cpu.StatusString() == "NvUbdIzc");
}

TEST_CASE("AndAbsoluteX ANDs the value at a 16-bit address plus X with the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x0F);
    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, base address = 0x0200
    cpu.SetXRegister(0x10);
    bus.WriteCpu(0x0210, 0x05); // value, result = 0x05

    cpu.AndAbsoluteX();

    CHECK(cpu.GetAccumulator() == 0x05);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("AndAbsoluteY ANDs the value at a 16-bit address plus Y with the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x0F);
    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x03); // high byte, base address = 0x0300
    cpu.SetYRegister(0x20);
    bus.WriteCpu(0x0320, 0x00); // value, result = 0x00

    cpu.AndAbsoluteY();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc");
}

TEST_CASE("AndIndirectX adds X to the zero page base, reads a pointer, then ANDs the value") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xFF);
    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x78); // pointer low byte
    bus.WriteCpu(0x16, 0x06); // pointer high byte, target address = 0x0678
    bus.WriteCpu(0x0678, 0x99); // value, result = 0x99

    cpu.AndIndirectX();

    CHECK(cpu.GetAccumulator() == 0x99);
    CHECK(cpu.StatusString() == "NvUbdIzc");
}

TEST_CASE("AndIndirectY reads a zero page pointer, adds Y, then ANDs the value") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xFF);
    bus.WriteCpu(0x00, 0x20); // zero page pointer location
    bus.WriteCpu(0x20, 0x00); // pointer low byte
    bus.WriteCpu(0x21, 0x02); // pointer high byte, base address = 0x0200
    cpu.SetYRegister(0x05);
    bus.WriteCpu(0x0205, 0x00); // value, result = 0x00

    cpu.AndIndirectY();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc");
}