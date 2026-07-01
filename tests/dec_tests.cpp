#include "../doctest.h"

#include "../src/Bus.h"
#include "../src/Cpu.h"

TEST_CASE("DecZeroPage decrements the value at the zero page address and stores the result") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // zero page address operand
    bus.WriteCpu(0x10, 0x02); // value at the zero page address

    cpu.DecZeroPage();

    CHECK(bus.ReadCpu(0x10) == 0x01);
    CHECK(cpu.StatusString() == "nvUbdIzc"); // Zero clear, Negative clear
}

TEST_CASE("DecZeroPage sets Zero when the result reaches 0x00") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // zero page address operand
    bus.WriteCpu(0x10, 0x01); // decrements to 0x00

    cpu.DecZeroPage();

    CHECK(bus.ReadCpu(0x10) == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc"); // Zero set, Negative clear
}

TEST_CASE("DecZeroPage sets Negative when the result wraps to 0xFF") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // zero page address operand
    bus.WriteCpu(0x10, 0x00); // wraps to 0xFF

    cpu.DecZeroPage();

    CHECK(bus.ReadCpu(0x10) == 0xFF);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Zero clear, Negative set
}

TEST_CASE("DecZeroPageX decrements the value at the zero page address plus X") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x80); // value at 0x10 + X

    cpu.DecZeroPageX();

    CHECK(bus.ReadCpu(0x15) == 0x7F);
    CHECK(cpu.StatusString() == "nvUbdIzc"); // Zero clear, Negative clear
}

TEST_CASE("DecZeroPageX wraps within the zero page instead of crossing into page 1") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xFF); // base operand
    cpu.SetXRegister(0x02);
    bus.WriteCpu(0x01, 0x03); // value at wrapped address (0xFF + 0x02) & 0xFF

    cpu.DecZeroPageX();

    CHECK(bus.ReadCpu(0x01) == 0x02);
}

TEST_CASE("DecAbsolute decrements the value at a 16-bit address and sets Negative") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, address = 0x0200
    bus.WriteCpu(0x0200, 0x00); // wraps to 0xFF

    cpu.DecAbsolute();

    CHECK(bus.ReadCpu(0x0200) == 0xFF);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Zero clear, Negative set
}

TEST_CASE("DecAbsoluteX decrements the value at a 16-bit address plus X") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, base address = 0x0200
    cpu.SetXRegister(0x10);
    bus.WriteCpu(0x0210, 0x01); // decrements to 0x00

    cpu.DecAbsoluteX();

    CHECK(bus.ReadCpu(0x0210) == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc"); // Zero set, Negative clear
}