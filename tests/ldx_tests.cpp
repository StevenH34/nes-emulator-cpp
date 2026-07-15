#include "doctest.h"

#include "../src/Bus.h"
#include "TestBus.h"
#include "../src/cpu/Cpu.h"

TEST_CASE("Ldx loads the X register and updates the Zero and Negative flags") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Ldx(0x00);
    CHECK(cpu.GetXRegister() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc"); // Zero flag set, Negative clear

    cpu.Ldx(0x80);
    CHECK(cpu.GetXRegister() == 0x80);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Negative flag set, Zero clear

    cpu.Ldx(0x01);
    CHECK(cpu.GetXRegister() == 0x01);
    CHECK(cpu.StatusString() == "nvUbdIzc"); // both flags clear
}

TEST_CASE("LdxImmediate loads the value following the opcode") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x42); // value

    cpu.LdxImmediate();

    CHECK(cpu.GetXRegister() == 0x42);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("LdxZeroPage loads the value at the zero page address") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // zero page address
    bus.WriteCpu(0x10, 0x37); // value

    cpu.LdxZeroPage();

    CHECK(cpu.GetXRegister() == 0x37);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("LdxZeroPageY loads the value at the zero page address plus Y") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetYRegister(0x05);
    bus.WriteCpu(0x15, 0x00); // value, sets Zero flag

    cpu.LdxZeroPageY();

    CHECK(cpu.GetXRegister() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc");
}

TEST_CASE("LdxAbsolute loads the value at a 16-bit address") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, address = 0x0200
    bus.WriteCpu(0x0200, 0x80); // value, sets Negative flag

    cpu.LdxAbsolute();

    CHECK(cpu.GetXRegister() == 0x80);
    CHECK(cpu.StatusString() == "NvUbdIzc");
}

TEST_CASE("LdxAbsoluteY loads the value at a 16-bit address plus Y") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x03); // high byte, base address = 0x0300
    cpu.SetYRegister(0x20);
    bus.WriteCpu(0x0320, 0x05); // value

    cpu.LdxAbsoluteY();

    CHECK(cpu.GetXRegister() == 0x05);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}
