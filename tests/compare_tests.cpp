#include "../doctest.h"

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