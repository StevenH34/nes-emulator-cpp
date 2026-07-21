#include "doctest.h"

#include "../../../src/core/Bus.h"
#include "TestBus.h"
#include "../../../src/core/cpu/Cpu.h"

TEST_CASE("Lda loads the accumulator and updates the Zero and Negative flags") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x00);
    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc"); // Zero flag set, Negative clear

    cpu.Lda(0x80);
    CHECK(cpu.GetAccumulator() == 0x80);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Negative flag set, Zero clear

    cpu.Lda(0x01);
    CHECK(cpu.GetAccumulator() == 0x01);
    CHECK(cpu.StatusString() == "nvUbdIzc"); // both flags clear
}

TEST_CASE("LdaImmediate loads the value following the opcode") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x42); // value

    cpu.LdaImmediate();

    CHECK(cpu.GetAccumulator() == 0x42);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("LdaZeroPage loads the value at the zero page address") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // zero page address
    bus.WriteCpu(0x10, 0x37); // value

    cpu.LdaZeroPage();

    CHECK(cpu.GetAccumulator() == 0x37);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("LdaZeroPageX loads the value at the zero page address plus X") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x00); // value, sets Zero flag

    cpu.LdaZeroPageX();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc");
}

TEST_CASE("LdaAbsolute loads the value at a 16-bit address") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, address = 0x0200
    bus.WriteCpu(0x0200, 0x80); // value, sets Negative flag

    cpu.LdaAbsolute();

    CHECK(cpu.GetAccumulator() == 0x80);
    CHECK(cpu.StatusString() == "NvUbdIzc");
}

TEST_CASE("LdaAbsoluteX loads the value at a 16-bit address plus X") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, base address = 0x0200
    cpu.SetXRegister(0x10);
    bus.WriteCpu(0x0210, 0x05); // value

    cpu.LdaAbsoluteX();

    CHECK(cpu.GetAccumulator() == 0x05);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("LdaAbsoluteY loads the value at a 16-bit address plus Y") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x03); // high byte, base address = 0x0300
    cpu.SetYRegister(0x20);
    bus.WriteCpu(0x0320, 0x05); // value

    cpu.LdaAbsoluteY();

    CHECK(cpu.GetAccumulator() == 0x05);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("LdaIndirectX adds X to the zero page base, reads a pointer, then loads the value") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x78); // pointer low byte
    bus.WriteCpu(0x16, 0x06); // pointer high byte, target address = 0x0678
    bus.WriteCpu(0x0678, 0x99); // value, sets Negative flag

    cpu.LdaIndirectX();

    CHECK(cpu.GetAccumulator() == 0x99);
    CHECK(cpu.StatusString() == "NvUbdIzc");
}

TEST_CASE("LdaIndirectY reads a zero page pointer, adds Y, then loads the value") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x20); // zero page pointer location
    bus.WriteCpu(0x20, 0x00); // pointer low byte
    bus.WriteCpu(0x21, 0x02); // pointer high byte, base address = 0x0200
    cpu.SetYRegister(0x05);
    bus.WriteCpu(0x0205, 0x00); // value, sets Zero flag

    cpu.LdaIndirectY();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc");
}