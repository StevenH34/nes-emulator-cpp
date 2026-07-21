#include "doctest.h"

#include "../../../src/core/Bus.h"
#include "TestBus.h"
#include "../../../src/core/cpu/Cpu.h"

TEST_CASE("IncZeroPage increments the value at the zero page address and stores the result") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // zero page address operand
    bus.WriteCpu(0x10, 0x01); // value at the zero page address

    cpu.IncZeroPage();

    CHECK(bus.ReadCpu(0x10) == 0x02);
    CHECK(cpu.StatusString() == "nvUbdIzc"); // Zero clear, Negative clear
}

TEST_CASE("IncZeroPage sets Zero when the result wraps to 0x00") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // zero page address operand
    bus.WriteCpu(0x10, 0xFF); // wraps to 0x00

    cpu.IncZeroPage();

    CHECK(bus.ReadCpu(0x10) == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc"); // Zero set, Negative clear
}

TEST_CASE("IncZeroPage sets Negative when bit 7 of the result is set") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // zero page address operand
    bus.WriteCpu(0x10, 0x7F); // increments to 0x80

    cpu.IncZeroPage();

    CHECK(bus.ReadCpu(0x10) == 0x80);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Zero clear, Negative set
}

TEST_CASE("IncZeroPageX increments the value at the zero page address plus X") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x7F); // value at 0x10 + X

    cpu.IncZeroPageX();

    CHECK(bus.ReadCpu(0x15) == 0x80);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Negative set
}

TEST_CASE("IncZeroPageX wraps within the zero page instead of crossing into page 1") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xFF); // base operand
    cpu.SetXRegister(0x02);
    bus.WriteCpu(0x01, 0x03); // value at wrapped address (0xFF + 0x02) & 0xFF

    cpu.IncZeroPageX();

    CHECK(bus.ReadCpu(0x01) == 0x04);
}

TEST_CASE("IncAbsolute increments the value at a 16-bit address and sets Zero") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, address = 0x0200
    bus.WriteCpu(0x0200, 0xFF); // wraps to 0x00

    cpu.IncAbsolute();

    CHECK(bus.ReadCpu(0x0200) == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc"); // Zero set, Negative clear
}

TEST_CASE("IncAbsoluteX increments the value at a 16-bit address plus X") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, base address = 0x0200
    cpu.SetXRegister(0x10);
    bus.WriteCpu(0x0210, 0x01);

    cpu.IncAbsoluteX();

    CHECK(bus.ReadCpu(0x0210) == 0x02);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}