#include "doctest.h"

#include "../src/Bus.h"
#include "TestBus.h"
#include "../src/Cpu.h"

TEST_CASE("EorImmediate XORs the value following the opcode with the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xFF);
    bus.WriteCpu(0x00, 0xFF); // value, result = 0x00

    cpu.EorImmediate();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc");
}

TEST_CASE("EorImmediate sets the Negative flag when the result has bit 7 set") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x0F);
    bus.WriteCpu(0x00, 0x80); // value, result = 0x8F

    cpu.EorImmediate();

    CHECK(cpu.GetAccumulator() == 0x8F);
    CHECK(cpu.StatusString() == "NvUbdIzc");
}

TEST_CASE("EorImmediate clears the Zero flag when the result is non-zero") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x0F);
    bus.WriteCpu(0x00, 0x01); // value, result = 0x0E

    cpu.EorImmediate();

    CHECK(cpu.GetAccumulator() == 0x0E);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("EorZeroPage XORs the value at the zero page address with the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xFF);
    bus.WriteCpu(0x00, 0x10); // zero page address
    bus.WriteCpu(0x10, 0xFF); // value, result = 0x00

    cpu.EorZeroPage();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc");
}

TEST_CASE("EorZeroPage sets the Negative flag when the result has bit 7 set") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x0F);
    bus.WriteCpu(0x00, 0x10); // zero page address
    bus.WriteCpu(0x10, 0x80); // value, result = 0x8F

    cpu.EorZeroPage();

    CHECK(cpu.GetAccumulator() == 0x8F);
    CHECK(cpu.StatusString() == "NvUbdIzc");
}

TEST_CASE("EorZeroPageX XORs the value at the zero page address plus X with the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xFF);
    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0xFF); // value, result = 0x00

    cpu.EorZeroPageX();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc");
}

TEST_CASE("EorZeroPageX sets the Negative flag when the result has bit 7 set") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x0F);
    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x80); // value, result = 0x8F

    cpu.EorZeroPageX();

    CHECK(cpu.GetAccumulator() == 0x8F);
    CHECK(cpu.StatusString() == "NvUbdIzc");
}

TEST_CASE("EorAbsolute XORs the value at a 16-bit address with the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xFF);
    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, address = 0x0200
    bus.WriteCpu(0x0200, 0xFF); // value, result = 0x00

    cpu.EorAbsolute();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc");
}

TEST_CASE("EorAbsolute sets the Negative flag when the result has bit 7 set") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x0F);
    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, address = 0x0200
    bus.WriteCpu(0x0200, 0x80); // value, result = 0x8F

    cpu.EorAbsolute();

    CHECK(cpu.GetAccumulator() == 0x8F);
    CHECK(cpu.StatusString() == "NvUbdIzc");
}

TEST_CASE("EorAbsoluteX XORs the value at a 16-bit address plus X with the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xFF);
    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, base address = 0x0200
    cpu.SetXRegister(0x10);
    bus.WriteCpu(0x0210, 0xFF); // value, result = 0x00

    cpu.EorAbsoluteX();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc");
}

TEST_CASE("EorAbsoluteX sets the Negative flag when the result has bit 7 set") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x0F);
    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, base address = 0x0200
    cpu.SetXRegister(0x10);
    bus.WriteCpu(0x0210, 0x80); // value, result = 0x8F

    cpu.EorAbsoluteX();

    CHECK(cpu.GetAccumulator() == 0x8F);
    CHECK(cpu.StatusString() == "NvUbdIzc");
}

TEST_CASE("EorAbsoluteY XORs the value at a 16-bit address plus Y with the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xFF);
    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x03); // high byte, base address = 0x0300
    cpu.SetYRegister(0x20);
    bus.WriteCpu(0x0320, 0xFF); // value, result = 0x00

    cpu.EorAbsoluteY();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc");
}

TEST_CASE("EorAbsoluteY sets the Negative flag when the result has bit 7 set") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x0F);
    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x03); // high byte, base address = 0x0300
    cpu.SetYRegister(0x20);
    bus.WriteCpu(0x0320, 0x80); // value, result = 0x8F

    cpu.EorAbsoluteY();

    CHECK(cpu.GetAccumulator() == 0x8F);
    CHECK(cpu.StatusString() == "NvUbdIzc");
}

TEST_CASE("EorIndirectX adds X to the zero page base, reads a pointer, then XORs the value") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xFF);
    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x78); // pointer low byte
    bus.WriteCpu(0x16, 0x06); // pointer high byte, target address = 0x0678
    bus.WriteCpu(0x0678, 0xFF); // value, result = 0x00

    cpu.EorIndirectX();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc");
}

TEST_CASE("EorIndirectX sets the Negative flag when the result has bit 7 set") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x0F);
    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x78); // pointer low byte
    bus.WriteCpu(0x16, 0x06); // pointer high byte, target address = 0x0678
    bus.WriteCpu(0x0678, 0x80); // value, result = 0x8F

    cpu.EorIndirectX();

    CHECK(cpu.GetAccumulator() == 0x8F);
    CHECK(cpu.StatusString() == "NvUbdIzc");
}

TEST_CASE("EorIndirectY reads a zero page pointer, adds Y, then XORs the value") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xFF);
    bus.WriteCpu(0x00, 0x20); // zero page pointer location
    bus.WriteCpu(0x20, 0x00); // pointer low byte
    bus.WriteCpu(0x21, 0x02); // pointer high byte, base address = 0x0200
    cpu.SetYRegister(0x05);
    bus.WriteCpu(0x0205, 0xFF); // value, result = 0x00

    cpu.EorIndirectY();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc");
}

TEST_CASE("EorIndirectY sets the Negative flag when the result has bit 7 set") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x0F);
    bus.WriteCpu(0x00, 0x20); // zero page pointer location
    bus.WriteCpu(0x20, 0x00); // pointer low byte
    bus.WriteCpu(0x21, 0x02); // pointer high byte, base address = 0x0200
    cpu.SetYRegister(0x05);
    bus.WriteCpu(0x0205, 0x80); // value, result = 0x8F

    cpu.EorIndirectY();

    CHECK(cpu.GetAccumulator() == 0x8F);
    CHECK(cpu.StatusString() == "NvUbdIzc");
}