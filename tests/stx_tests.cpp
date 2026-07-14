#include "doctest.h"

#include "../src/Bus.h"
#include "TestBus.h"
#include "../src/Cpu.h"

TEST_CASE("StxZeroPage writes the X register to the zero page address") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // zero page address
    cpu.SetXRegister(0x42);

    cpu.StxZeroPage();

    CHECK(bus.ReadCpu(0x10) == 0x42);
}

TEST_CASE("StxZeroPageY writes the X register to the zero page address plus Y") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetYRegister(0x05);
    cpu.SetXRegister(0x37);

    cpu.StxZeroPageY();

    CHECK(bus.ReadCpu(0x15) == 0x37);
}

TEST_CASE("StxAbsolute writes the X register to a 16-bit address") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, address = 0x0200
    cpu.SetXRegister(0x7F);

    cpu.StxAbsolute();

    CHECK(bus.ReadCpu(0x0200) == 0x7F);
}