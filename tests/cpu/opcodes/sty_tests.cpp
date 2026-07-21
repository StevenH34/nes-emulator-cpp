#include "doctest.h"

#include "../../../src/core/Bus.h"
#include "TestBus.h"
#include "../../../src/core/cpu/Cpu.h"

TEST_CASE("StyZeroPage writes the Y register to the zero page address") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // zero page address
    cpu.SetYRegister(0x42);

    cpu.StyZeroPage();

    CHECK(bus.ReadCpu(0x10) == 0x42);
}

TEST_CASE("StyZeroPageX writes the Y register to the zero page address plus X") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetXRegister(0x05);
    cpu.SetYRegister(0x37);

    cpu.StyZeroPageX();

    CHECK(bus.ReadCpu(0x15) == 0x37);
}

TEST_CASE("StyAbsolute writes the Y register to a 16-bit address") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, address = 0x0200
    cpu.SetYRegister(0x7F);

    cpu.StyAbsolute();

    CHECK(bus.ReadCpu(0x0200) == 0x7F);
}