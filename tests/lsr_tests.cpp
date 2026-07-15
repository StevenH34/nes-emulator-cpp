#include "doctest.h"

#include "../src/Bus.h"
#include "TestBus.h"
#include "../src/Cpu.h"

TEST_CASE("LsrAccumulator shifts bits right and clears Carry and Zero") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x02); // 0000 0010

    cpu.LsrAccumulator();

    CHECK(cpu.GetAccumulator() == 0x01); // 0000 0001
    CHECK(cpu.StatusString() == "nvUbdIzc"); // Carry clear, Zero clear, Negative clear
}

TEST_CASE("LsrAccumulator sets Carry when bit 0 is shifted out") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x03); // 0000 0011

    cpu.LsrAccumulator();

    CHECK(cpu.GetAccumulator() == 0x01); // 0000 0001, bit 0 dropped
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero clear, Negative clear
}

TEST_CASE("LsrAccumulator sets Zero when the result is 0x00") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x01); // 0000 0001

    cpu.LsrAccumulator();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("LsrAccumulator always clears Negative because bit 7 becomes 0") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x81); // 1000 0001

    cpu.LsrAccumulator();

    CHECK(cpu.GetAccumulator() == 0x40); // 0100 0000
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set (bit 0 dropped), Negative clear
}

TEST_CASE("Lsr returns the shifted value without touching memory or the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    CHECK(cpu.Lsr(0x03) == 0x01); // 0000 0011 -> 0000 0001, bit 0 dropped
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero clear, Negative clear
    CHECK(cpu.GetAccumulator() == 0x00);
}

TEST_CASE("LsrZeroPage shifts the value at the zero page address and stores the result") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // zero page address operand
    bus.WriteCpu(0x10, 0x02); // value at the zero page address

    cpu.LsrZeroPage();

    CHECK(bus.ReadCpu(0x10) == 0x01);
    CHECK(cpu.StatusString() == "nvUbdIzc"); // Carry clear, Zero clear, Negative clear
}

TEST_CASE("LsrZeroPage sets Carry when bit 0 is shifted out") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // zero page address operand
    bus.WriteCpu(0x10, 0x01); // 0000 0001

    cpu.LsrZeroPage();

    CHECK(bus.ReadCpu(0x10) == 0x00); // bit 0 dropped
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("LsrZeroPageX shifts the value at the zero page address plus X") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x80); // value at 0x10 + X

    cpu.LsrZeroPageX();

    CHECK(bus.ReadCpu(0x15) == 0x40);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("LsrZeroPageX wraps within the zero page instead of crossing into page 1") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xFF); // base operand
    cpu.SetXRegister(0x02);
    bus.WriteCpu(0x01, 0x06); // value at wrapped address (0xFF + 0x02) & 0xFF

    cpu.LsrZeroPageX();

    CHECK(bus.ReadCpu(0x01) == 0x03);
}

TEST_CASE("LsrAbsolute shifts the value at a 16-bit address and sets Zero and Carry") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, address = 0x0200
    bus.WriteCpu(0x0200, 0x01); // 0000 0001

    cpu.LsrAbsolute();

    CHECK(bus.ReadCpu(0x0200) == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("LsrAbsoluteX shifts the value at a 16-bit address plus X") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, base address = 0x0200
    cpu.SetXRegister(0x10);
    bus.WriteCpu(0x0210, 0x04);

    cpu.LsrAbsoluteX();

    CHECK(bus.ReadCpu(0x0210) == 0x02);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}