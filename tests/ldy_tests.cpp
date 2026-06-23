#include "../doctest.h"

#include "../src/Bus.h"
#include "../src/Cpu.h"

TEST_CASE("Ldy loads the Y register and updates the Zero and Negative flags") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Ldy(0x00);
    CHECK(cpu.GetYRegister() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc"); // Zero flag set, Negative clear

    cpu.Ldy(0x80);
    CHECK(cpu.GetYRegister() == 0x80);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Negative flag set, Zero clear

    cpu.Ldy(0x01);
    CHECK(cpu.GetYRegister() == 0x01);
    CHECK(cpu.StatusString() == "nvUbdIzc"); // both flags clear
}

TEST_CASE("LdyImmediate loads the value following the opcode") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x42); // value

    cpu.LdyImmediate();

    CHECK(cpu.GetYRegister() == 0x42);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("LdyZeroPage loads the value at the zero page address") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // zero page address
    bus.WriteCpu(0x10, 0x37); // value

    cpu.LdyZeroPage();

    CHECK(cpu.GetYRegister() == 0x37);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("LdyZeroPageX loads the value at the zero page address plus X") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x00); // value, sets Zero flag

    cpu.LdyZeroPageX();

    CHECK(cpu.GetYRegister() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc");
}

TEST_CASE("LdyAbsolute loads the value at a 16-bit address") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, address = 0x0200
    bus.WriteCpu(0x0200, 0x80); // value, sets Negative flag

    cpu.LdyAbsolute();

    CHECK(cpu.GetYRegister() == 0x80);
    CHECK(cpu.StatusString() == "NvUbdIzc");
}

TEST_CASE("LdyAbsoluteX loads the value at a 16-bit address plus X") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x03); // high byte, base address = 0x0300
    cpu.SetXRegister(0x20);
    bus.WriteCpu(0x0320, 0x05); // value

    cpu.LdyAbsoluteX();

    CHECK(cpu.GetYRegister() == 0x05);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}