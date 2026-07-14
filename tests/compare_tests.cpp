#include "doctest.h"

#include "../src/Bus.h"
#include "../src/Cpu.h"

TEST_CASE("Compare sets Carry and clears Zero and Negative when register value is greater than operand") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Compare(0x10, 0x05); // result = 0x0B

    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero and Negative clear
}

TEST_CASE("Compare sets Carry and Zero when register value equals operand") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Compare(0x42, 0x42); // result = 0x00

    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("Compare clears Carry and sets Negative when register value is less than operand") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Compare(0x05, 0x10); // result = 0xF5 (borrow)

    CHECK(cpu.StatusString() == "NvUbdIzc"); // Carry clear, Zero clear, Negative set
}

TEST_CASE("Compare does not modify the accumulator, X, or Y registers") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x42);
    cpu.SetXRegister(0x10);
    cpu.SetYRegister(0x20);

    cpu.Compare(0x10, 0x05);

    CHECK(cpu.GetAccumulator() == 0x42);
    CHECK(cpu.GetXRegister() == 0x10);
    CHECK(cpu.GetYRegister() == 0x20);
}

TEST_CASE("CmpImmediate compares the accumulator with the value following the opcode") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x42);
    bus.WriteCpu(0x00, 0x42); // value, equal to accumulator

    cpu.CmpImmediate();

    CHECK(cpu.GetProgramCounter() == 0x0001); // one operand byte consumed
    CHECK(cpu.GetAccumulator() == 0x42); // accumulator left unchanged
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("CmpImmediate clears Carry when the accumulator is less than the operand") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x05);
    bus.WriteCpu(0x00, 0x10);

    cpu.CmpImmediate();

    CHECK(cpu.StatusString() == "NvUbdIzc"); // Carry clear, Zero clear, Negative set
}

TEST_CASE("CpxImmediate compares the X register with the value following the opcode") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.SetXRegister(0x42);
    bus.WriteCpu(0x00, 0x42); // value, equal to X register

    cpu.CpxImmediate();

    CHECK(cpu.GetProgramCounter() == 0x0001);
    CHECK(cpu.GetXRegister() == 0x42); // X register left unchanged
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("CpxImmediate clears Carry when the X register is less than the operand") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x00, 0x10);

    cpu.CpxImmediate();

    CHECK(cpu.StatusString() == "NvUbdIzc"); // Carry clear, Zero clear, Negative set
}

TEST_CASE("CpxZeroPage compares the X register with the value at the zero page address") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.SetXRegister(0x50);
    bus.WriteCpu(0x00, 0x30); // zero page address
    bus.WriteCpu(0x30, 0x20); // value, 0x50 > 0x20

    cpu.CpxZeroPage();

    CHECK(cpu.GetProgramCounter() == 0x0001);
    CHECK(cpu.GetXRegister() == 0x50); // X register left unchanged
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero and Negative clear
}

TEST_CASE("CpxZeroPage clears Carry when the X register is less than the zero page value") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.SetXRegister(0x10);
    bus.WriteCpu(0x00, 0x40); // zero page address
    bus.WriteCpu(0x40, 0x80); // value, 0x10 < 0x80

    cpu.CpxZeroPage();

    CHECK(cpu.StatusString() == "NvUbdIzc"); // Carry clear, Zero clear, Negative set
}

TEST_CASE("CpxAbsolute compares the X register with the value at a 16-bit address") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.SetXRegister(0x50);
    bus.WriteCpu(0x00, 0x00); // low byte of address
    bus.WriteCpu(0x01, 0x03); // high byte of address, target = 0x0300
    bus.WriteCpu(0x0300, 0x01); // value, 0x50 - 0x01 = 0x4F (bit 7 clear)

    cpu.CpxAbsolute();

    CHECK(cpu.GetProgramCounter() == 0x0002);
    CHECK(cpu.GetXRegister() == 0x50); // X register left unchanged
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero and Negative clear
}

TEST_CASE("CpxAbsolute clears Carry when the X register is less than the absolute value") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x00, 0x00); // low byte of address
    bus.WriteCpu(0x01, 0x04); // high byte of address, target = 0x0400
    bus.WriteCpu(0x0400, 0x10); // value, 0x05 - 0x10 = 0xF5 (bit 7 set)

    cpu.CpxAbsolute();

    CHECK(cpu.StatusString() == "NvUbdIzc"); // Carry clear, Zero clear, Negative set
}

TEST_CASE("CpyImmediate compares the Y register with the value following the opcode") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.SetYRegister(0x42);
    bus.WriteCpu(0x00, 0x42); // value, equal to Y register

    cpu.CpyImmediate();

    CHECK(cpu.GetProgramCounter() == 0x0001);
    CHECK(cpu.GetYRegister() == 0x42); // Y register left unchanged
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("CpyImmediate clears Carry when the Y register is less than the operand") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.SetYRegister(0x05);
    bus.WriteCpu(0x00, 0x10);

    cpu.CpyImmediate();

    CHECK(cpu.StatusString() == "NvUbdIzc"); // Carry clear, Zero clear, Negative set
}

TEST_CASE("CmpZeroPage compares the accumulator with the value at the zero page address") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x50);
    bus.WriteCpu(0x00, 0x30); // zero page address
    bus.WriteCpu(0x30, 0x20); // value, 0x50 > 0x20

    cpu.CmpZeroPage();

    CHECK(cpu.GetProgramCounter() == 0x0001);
    CHECK(cpu.GetAccumulator() == 0x50); // accumulator left unchanged
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero and Negative clear
}

TEST_CASE("CmpZeroPage clears Carry when the accumulator is less than the zero page value") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x10);
    bus.WriteCpu(0x00, 0x40); // zero page address
    bus.WriteCpu(0x40, 0x80); // value, 0x10 < 0x80

    cpu.CmpZeroPage();

    CHECK(cpu.StatusString() == "NvUbdIzc"); // Carry clear, Zero clear, Negative set
}

TEST_CASE("CmpZeroPageX compares the accumulator with the value at the zero page address plus X") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x42);
    bus.WriteCpu(0x00, 0x10); // base zero page address
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x42); // value at 0x10 + 0x05, equal to accumulator

    cpu.CmpZeroPageX();

    CHECK(cpu.GetProgramCounter() == 0x0001);
    CHECK(cpu.GetAccumulator() == 0x42); // accumulator left unchanged
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("CmpZeroPageX clears Carry when the accumulator is less than the indexed zero page value") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x05);
    bus.WriteCpu(0x00, 0x20); // base zero page address
    cpu.SetXRegister(0x02);
    bus.WriteCpu(0x22, 0x10); // value at 0x20 + 0x02, 0x05 - 0x10 = 0xF5 (bit 7 set)

    cpu.CmpZeroPageX();

    CHECK(cpu.StatusString() == "NvUbdIzc"); // Carry clear, Zero clear, Negative set
}

TEST_CASE("CmpAbsolute compares the accumulator with the value at a 16-bit address") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x50);
    bus.WriteCpu(0x00, 0x00); // low byte of address
    bus.WriteCpu(0x01, 0x03); // high byte of address, target = 0x0300
    bus.WriteCpu(0x0300, 0x01); // value, 0x50 - 0x01 = 0x4F (bit 7 clear)

    cpu.CmpAbsolute();

    CHECK(cpu.GetProgramCounter() == 0x0002);
    CHECK(cpu.GetAccumulator() == 0x50); // accumulator left unchanged
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero and Negative clear
}

TEST_CASE("CmpAbsolute clears Carry when the accumulator is less than the absolute value") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x05);
    bus.WriteCpu(0x00, 0x00); // low byte of address
    bus.WriteCpu(0x01, 0x04); // high byte of address, target = 0x0400
    bus.WriteCpu(0x0400, 0x10); // value, 0x05 - 0x10 = 0xF5 (bit 7 set)

    cpu.CmpAbsolute();

    CHECK(cpu.StatusString() == "NvUbdIzc"); // Carry clear, Zero clear, Negative set
}

TEST_CASE("CmpAbsoluteX compares the accumulator with the value at a 16-bit address plus X") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x30);
    bus.WriteCpu(0x00, 0x00); // low byte of base address
    bus.WriteCpu(0x01, 0x02); // high byte of base address, base = 0x0200
    cpu.SetXRegister(0x10);
    bus.WriteCpu(0x0210, 0x30); // value at 0x0200 + 0x10, equal to accumulator

    cpu.CmpAbsoluteX();

    CHECK(cpu.GetProgramCounter() == 0x0002);
    CHECK(cpu.GetAccumulator() == 0x30); // accumulator left unchanged
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("CmpAbsoluteX clears Carry when the accumulator is less than the indexed absolute value") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x05);
    bus.WriteCpu(0x00, 0x00); // low byte of base address
    bus.WriteCpu(0x01, 0x05); // high byte of base address, base = 0x0500
    cpu.SetXRegister(0x08);
    bus.WriteCpu(0x0508, 0x10); // value at 0x0500 + 0x08, 0x05 - 0x10 = 0xF5 (bit 7 set)

    cpu.CmpAbsoluteX();

    CHECK(cpu.StatusString() == "NvUbdIzc"); // Carry clear, Zero clear, Negative set
}

TEST_CASE("CmpAbsoluteY compares the accumulator with the value at a 16-bit address plus Y") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x80);
    bus.WriteCpu(0x00, 0x00); // low byte of base address
    bus.WriteCpu(0x01, 0x06); // high byte of base address, base = 0x0600
    cpu.SetYRegister(0x20);
    bus.WriteCpu(0x0620, 0x40); // value at 0x0600 + 0x20, 0x80 > 0x40

    cpu.CmpAbsoluteY();

    CHECK(cpu.GetProgramCounter() == 0x0002);
    CHECK(cpu.GetAccumulator() == 0x80); // accumulator left unchanged
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero and Negative clear
}

TEST_CASE("CmpAbsoluteY clears Carry when the accumulator is less than the Y-indexed absolute value") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x05);
    bus.WriteCpu(0x00, 0x00); // low byte of base address
    bus.WriteCpu(0x01, 0x07); // high byte of base address, base = 0x0700
    cpu.SetYRegister(0x05);
    bus.WriteCpu(0x0705, 0x10); // value at 0x0700 + 0x05, 0x05 - 0x10 = 0xF5 (bit 7 set)

    cpu.CmpAbsoluteY();

    CHECK(cpu.StatusString() == "NvUbdIzc"); // Carry clear, Zero clear, Negative set
}

TEST_CASE("CmpIndirectX adds X to the zero page base, reads a pointer, then compares with the accumulator") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x99);
    bus.WriteCpu(0x00, 0x10); // base zero page operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x00); // pointer low byte
    bus.WriteCpu(0x16, 0x03); // pointer high byte, target address = 0x0300
    bus.WriteCpu(0x0300, 0x99); // value, equal to accumulator

    cpu.CmpIndirectX();

    CHECK(cpu.GetProgramCounter() == 0x0001);
    CHECK(cpu.GetAccumulator() == 0x99); // accumulator left unchanged
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("CmpIndirectX clears Carry when the accumulator is less than the indirectly addressed value") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x05);
    bus.WriteCpu(0x00, 0x20); // base zero page operand
    cpu.SetXRegister(0x03);
    bus.WriteCpu(0x23, 0x00); // pointer low byte
    bus.WriteCpu(0x24, 0x04); // pointer high byte, target address = 0x0400
    bus.WriteCpu(0x0400, 0x10); // value, 0x05 - 0x10 = 0xF5 (bit 7 set)

    cpu.CmpIndirectX();

    CHECK(cpu.StatusString() == "NvUbdIzc"); // Carry clear, Zero clear, Negative set
}

TEST_CASE("CmpIndirectY reads a zero page pointer, adds Y, then compares with the accumulator") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x77);
    bus.WriteCpu(0x00, 0x30); // zero page pointer location
    bus.WriteCpu(0x30, 0x00); // pointer low byte
    bus.WriteCpu(0x31, 0x05); // pointer high byte, base address = 0x0500
    cpu.SetYRegister(0x10);
    bus.WriteCpu(0x0510, 0x11); // value at 0x0500 + 0x10, 0x77 > 0x11

    cpu.CmpIndirectY();

    CHECK(cpu.GetProgramCounter() == 0x0001);
    CHECK(cpu.GetAccumulator() == 0x77); // accumulator left unchanged
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero and Negative clear
}

TEST_CASE("CmpIndirectY clears Carry when the accumulator is less than the Y-offset indirect value") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x05);
    bus.WriteCpu(0x00, 0x40); // zero page pointer location
    bus.WriteCpu(0x40, 0x00); // pointer low byte
    bus.WriteCpu(0x41, 0x06); // pointer high byte, base address = 0x0600
    cpu.SetYRegister(0x08);
    bus.WriteCpu(0x0608, 0x10); // value at 0x0600 + 0x08, 0x05 - 0x10 = 0xF5 (bit 7 set)

    cpu.CmpIndirectY();

    CHECK(cpu.StatusString() == "NvUbdIzc"); // Carry clear, Zero clear, Negative set
}

TEST_CASE("CpyZeroPage compares the Y register with the value at the zero page address") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.SetYRegister(0x50);
    bus.WriteCpu(0x00, 0x30); // zero page address
    bus.WriteCpu(0x30, 0x20); // value, 0x50 > 0x20

    cpu.CpyZeroPage();

    CHECK(cpu.GetProgramCounter() == 0x0001);
    CHECK(cpu.GetYRegister() == 0x50); // Y register left unchanged
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero and Negative clear
}

TEST_CASE("CpyZeroPage clears Carry when the Y register is less than the zero page value") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.SetYRegister(0x10);
    bus.WriteCpu(0x00, 0x40); // zero page address
    bus.WriteCpu(0x40, 0x80); // value, 0x10 < 0x80

    cpu.CpyZeroPage();

    CHECK(cpu.StatusString() == "NvUbdIzc"); // Carry clear, Zero clear, Negative set
}

TEST_CASE("CpyAbsolute compares the Y register with the value at a 16-bit address") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.SetYRegister(0x50);
    bus.WriteCpu(0x00, 0x00); // low byte of address
    bus.WriteCpu(0x01, 0x03); // high byte of address, target = 0x0300
    bus.WriteCpu(0x0300, 0x01); // value, 0x50 - 0x01 = 0x4F (bit 7 clear)

    cpu.CpyAbsolute();

    CHECK(cpu.GetProgramCounter() == 0x0002);
    CHECK(cpu.GetYRegister() == 0x50); // Y register left unchanged
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero and Negative clear
}

TEST_CASE("CpyAbsolute clears Carry when the Y register is less than the absolute value") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.SetYRegister(0x05);
    bus.WriteCpu(0x00, 0x00); // low byte of address
    bus.WriteCpu(0x01, 0x04); // high byte of address, target = 0x0400
    bus.WriteCpu(0x0400, 0x10); // value, 0x05 - 0x10 = 0xF5 (bit 7 set)

    cpu.CpyAbsolute();

    CHECK(cpu.StatusString() == "NvUbdIzc"); // Carry clear, Zero clear, Negative set
}