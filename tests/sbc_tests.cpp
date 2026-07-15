#include "doctest.h"

#include "../src/Bus.h"
#include "TestBus.h"
#include "../src/cpu/Cpu.h"

TEST_CASE("Sbc subtracts the value from the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x50);
    cpu.SetCFlag(true); // no borrow in

    cpu.Sbc(0x30);

    CHECK(cpu.GetAccumulator() == 0x20);
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set (no borrow), no overflow
}

TEST_CASE("Sbc includes the Carry flag as inverted borrow") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x50);
    cpu.SetCFlag(false); // borrow in, so result is A - value - 1

    cpu.Sbc(0x30);

    CHECK(cpu.GetAccumulator() == 0x1F); // 0x50 - 0x30 - 1
    CHECK(cpu.StatusString() == "nvUbdIzC");
}

TEST_CASE("Sbc clears Carry when a borrow occurs") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x20);
    cpu.SetCFlag(true); // no borrow in

    cpu.Sbc(0x30); // 0x20 - 0x30 borrows, result = 0xF0

    CHECK(cpu.GetAccumulator() == 0xF0);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Carry clear (borrow occurred), Negative set
}

TEST_CASE("Sbc sets Zero when the result is 0x00") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x50);
    cpu.SetCFlag(true);

    cpu.Sbc(0x50);

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Zero set, Carry set (no borrow)
}

TEST_CASE("Sbc sets Negative when bit 7 of the result is set") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x50);
    cpu.SetCFlag(true);

    cpu.Sbc(0x70); // 0x50 - 0x70 = -0x20 = 0xE0

    CHECK(cpu.GetAccumulator() == 0xE0);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Negative set, Carry clear (borrow)
}

TEST_CASE("Sbc sets Overflow when positive minus negative produces a negative result") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x50); // +80
    cpu.SetCFlag(true);

    cpu.Sbc(0xB0); // -80; +80 - (-80) should be +160, wraps into negative range

    CHECK(cpu.GetAccumulator() == 0xA0);
    CHECK(cpu.StatusString() == "NVUbdIzc"); // Overflow set, Negative set, Carry clear
}

TEST_CASE("Sbc sets Overflow when negative minus positive produces a positive result") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xD0); // -48
    cpu.SetCFlag(true);

    cpu.Sbc(0x70); // +112; -48 - 112 = -160, wraps into positive range

    CHECK(cpu.GetAccumulator() == 0x60);
    CHECK(cpu.StatusString() == "nVUbdIzC"); // Overflow set, Carry set, Negative clear
}

TEST_CASE("Sbc does not set Overflow when operands have the same sign") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x50); // +80
    cpu.SetCFlag(true);

    cpu.Sbc(0x30); // +48; no signed overflow

    CHECK(cpu.GetAccumulator() == 0x20);
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Overflow clear
}

TEST_CASE("SbcImmediate subtracts the value following the opcode from the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x50);
    cpu.SetCFlag(true);
    bus.WriteCpu(0x00, 0x30);

    cpu.SbcImmediate();

    CHECK(cpu.GetProgramCounter() == 0x0001); // one operand byte consumed
    CHECK(cpu.GetAccumulator() == 0x20);
    CHECK(cpu.StatusString() == "nvUbdIzC");
}

TEST_CASE("SbcImmediate clears Carry and sets Zero when result wraps to 0x00 with borrow") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x01);
    cpu.SetCFlag(true);
    bus.WriteCpu(0x00, 0x01);

    cpu.SbcImmediate();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Zero set, Carry set (no borrow)
}

TEST_CASE("SbcZeroPage subtracts the value at the zero page address from the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x50);
    cpu.SetCFlag(true);
    bus.WriteCpu(0x00, 0x20); // zero page address
    bus.WriteCpu(0x20, 0x30); // value

    cpu.SbcZeroPage();

    CHECK(cpu.GetProgramCounter() == 0x0001); // one operand byte consumed
    CHECK(cpu.GetAccumulator() == 0x20);
    CHECK(cpu.StatusString() == "nvUbdIzC");
}

TEST_CASE("SbcZeroPageX subtracts the value at the zero page address plus X from the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x50);
    cpu.SetCFlag(true);
    bus.WriteCpu(0x00, 0x10); // base zero page address
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x30); // value at base+X

    cpu.SbcZeroPageX();

    CHECK(cpu.GetProgramCounter() == 0x0001); // one operand byte consumed
    CHECK(cpu.GetAccumulator() == 0x20);
    CHECK(cpu.StatusString() == "nvUbdIzC");
}

TEST_CASE("SbcAbsolute subtracts the value at a 16-bit address from the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x50);
    cpu.SetCFlag(true);
    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, address = 0x0200
    bus.WriteCpu(0x0200, 0x30); // value

    cpu.SbcAbsolute();

    CHECK(cpu.GetProgramCounter() == 0x0002); // two operand bytes consumed
    CHECK(cpu.GetAccumulator() == 0x20);
    CHECK(cpu.StatusString() == "nvUbdIzC");
}

TEST_CASE("SbcAbsoluteX subtracts the value at a 16-bit address plus X from the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x50);
    cpu.SetCFlag(true);
    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, base address = 0x0200
    cpu.SetXRegister(0x10);
    bus.WriteCpu(0x0210, 0x30); // value at base+X

    cpu.SbcAbsoluteX();

    CHECK(cpu.GetProgramCounter() == 0x0002); // two operand bytes consumed
    CHECK(cpu.GetAccumulator() == 0x20);
    CHECK(cpu.StatusString() == "nvUbdIzC");
}

TEST_CASE("SbcAbsoluteY subtracts the value at a 16-bit address plus Y from the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x50);
    cpu.SetCFlag(true);
    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, base address = 0x0200
    cpu.SetYRegister(0x10);
    bus.WriteCpu(0x0210, 0x30); // value at base+Y

    cpu.SbcAbsoluteY();

    CHECK(cpu.GetProgramCounter() == 0x0002); // two operand bytes consumed
    CHECK(cpu.GetAccumulator() == 0x20);
    CHECK(cpu.StatusString() == "nvUbdIzC");
}

TEST_CASE("SbcIndirectX adds X to the zero page base, reads a pointer, then subtracts the value") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x50);
    cpu.SetCFlag(true);
    bus.WriteCpu(0x00, 0x10); // base zero page operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x78); // pointer low byte
    bus.WriteCpu(0x16, 0x06); // pointer high byte, target address = 0x0678
    bus.WriteCpu(0x0678, 0x30); // value

    cpu.SbcIndirectX();

    CHECK(cpu.GetProgramCounter() == 0x0001); // one operand byte consumed
    CHECK(cpu.GetAccumulator() == 0x20);
    CHECK(cpu.StatusString() == "nvUbdIzC");
}

TEST_CASE("SbcIndirectY reads a zero page pointer, adds Y, then subtracts the value") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x50);
    cpu.SetCFlag(true);
    bus.WriteCpu(0x00, 0x20); // zero page pointer location
    bus.WriteCpu(0x20, 0x00); // pointer low byte
    bus.WriteCpu(0x21, 0x02); // pointer high byte, base address = 0x0200
    cpu.SetYRegister(0x05);
    bus.WriteCpu(0x0205, 0x30); // value at base+Y

    cpu.SbcIndirectY();

    CHECK(cpu.GetProgramCounter() == 0x0001); // one operand byte consumed
    CHECK(cpu.GetAccumulator() == 0x20);
    CHECK(cpu.StatusString() == "nvUbdIzC");
}