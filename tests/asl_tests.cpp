#include "doctest.h"

#include "../src/core/Bus.h"
#include "TestBus.h"
#include "../src/core/cpu/Cpu.h"

TEST_CASE("AslAccumulator shifts bits left and clears Carry, Zero, and Negative") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x01); // 0000 0001

    cpu.AslAccumulator();

    CHECK(cpu.GetAccumulator() == 0x02); // 0000 0010
    CHECK(cpu.StatusString() == "nvUbdIzc"); // Carry clear, Zero clear, Negative clear
}

TEST_CASE("AslAccumulator sets Carry when bit 7 is shifted out") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x81); // 1000 0001

    cpu.AslAccumulator();

    CHECK(cpu.GetAccumulator() == 0x02); // 0000 0010, bit 7 dropped
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero clear, Negative clear
}

TEST_CASE("AslAccumulator sets Zero when the result is 0x00") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x80); // 1000 0000

    cpu.AslAccumulator();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("AslAccumulator sets Negative when bit 7 of the result is set") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x40); // 0100 0000

    cpu.AslAccumulator();

    CHECK(cpu.GetAccumulator() == 0x80); // 1000 0000
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Carry clear, Zero clear, Negative set
}

TEST_CASE("Asl returns the shifted value without touching memory or the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    CHECK(cpu.Asl(0x81) == 0x02); // 1000 0001 -> 0000 0010, bit 7 dropped
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero clear, Negative clear
    CHECK(cpu.GetAccumulator() == 0x00);
}

TEST_CASE("AslZeroPage shifts the value at the zero page address and stores the result") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // zero page address operand
    bus.WriteCpu(0x10, 0x01); // value at the zero page address

    cpu.AslZeroPage();

    CHECK(bus.ReadCpu(0x10) == 0x02);
    CHECK(cpu.StatusString() == "nvUbdIzc"); // Carry clear, Zero clear, Negative clear
}

TEST_CASE("AslZeroPage sets Carry when bit 7 is shifted out") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // zero page address operand
    bus.WriteCpu(0x10, 0x81); // 1000 0001

    cpu.AslZeroPage();

    CHECK(bus.ReadCpu(0x10) == 0x02); // bit 7 dropped
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero clear, Negative clear
}

TEST_CASE("AslZeroPageX shifts the value at the zero page address plus X") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x40); // value at 0x10 + X

    cpu.AslZeroPageX();

    CHECK(bus.ReadCpu(0x15) == 0x80);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Negative set
}

TEST_CASE("AslZeroPageX wraps within the zero page instead of crossing into page 1") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xFF); // base operand
    cpu.SetXRegister(0x02);
    bus.WriteCpu(0x01, 0x03); // value at wrapped address (0xFF + 0x02) & 0xFF

    cpu.AslZeroPageX();

    CHECK(bus.ReadCpu(0x01) == 0x06);
}

TEST_CASE("AslAbsolute shifts the value at a 16-bit address and sets Zero and Carry") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, address = 0x0200
    bus.WriteCpu(0x0200, 0x80); // 1000 0000

    cpu.AslAbsolute();

    CHECK(bus.ReadCpu(0x0200) == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("AslAbsoluteX shifts the value at a 16-bit address plus X") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, base address = 0x0200
    cpu.SetXRegister(0x10);
    bus.WriteCpu(0x0210, 0x01);

    cpu.AslAbsoluteX();

    CHECK(bus.ReadCpu(0x0210) == 0x02);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}