#include "doctest.h"

#include "../src/Bus.h"
#include "TestBus.h"
#include "../src/cpu/Cpu.h"

TEST_CASE("RorAccumulator rotates bits right and shifts in a clear Carry") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x02); // 0000 0010

    cpu.RorAccumulator();

    CHECK(cpu.GetAccumulator() == 0x01); // 0000 0001
    CHECK(cpu.StatusString() == "nvUbdIzc"); // Carry clear, Zero clear, Negative clear
}

TEST_CASE("RorAccumulator shifts in a set Carry as bit 7") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x02); // 0000 0010
    cpu.SetCFlag(true);

    cpu.RorAccumulator();

    CHECK(cpu.GetAccumulator() == 0x81); // 1000 0001, old Carry becomes bit 7
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Carry clear, Zero clear, Negative set
}

TEST_CASE("RorAccumulator sets Carry when bit 0 is rotated out") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x03); // 0000 0011

    cpu.RorAccumulator();

    CHECK(cpu.GetAccumulator() == 0x01); // 0000 0001, bit 0 moved to Carry
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero clear, Negative clear
}

TEST_CASE("RorAccumulator sets Zero when the result is 0x00") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x01); // 0000 0001

    cpu.RorAccumulator();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("RorAccumulator sets Negative when the old Carry rotates into bit 7") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x00);
    cpu.SetCFlag(true);

    cpu.RorAccumulator();

    CHECK(cpu.GetAccumulator() == 0x80); // 1000 0000
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Carry clear, Zero clear, Negative set
}

TEST_CASE("Ror returns the rotated value without touching memory or the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    CHECK(cpu.Ror(0x03) == 0x01); // 0000 0011 -> 0000 0001, bit 0 moved to Carry
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero clear, Negative clear
    CHECK(cpu.GetAccumulator() == 0x00);
}

TEST_CASE("RorZeroPage rotates the value at the zero page address and stores the result") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // zero page address operand
    bus.WriteCpu(0x10, 0x02); // value at the zero page address

    cpu.RorZeroPage();

    CHECK(bus.ReadCpu(0x10) == 0x01);
    CHECK(cpu.StatusString() == "nvUbdIzc"); // Carry clear, Zero clear, Negative clear
}

TEST_CASE("RorZeroPage sets Carry when bit 0 is rotated out") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // zero page address operand
    bus.WriteCpu(0x10, 0x01); // 0000 0001

    cpu.RorZeroPage();

    CHECK(bus.ReadCpu(0x10) == 0x00); // bit 0 moved to Carry
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("RorZeroPageX rotates the value at the zero page address plus X") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x02); // value at 0x10 + X
    cpu.SetCFlag(true);

    cpu.RorZeroPageX();

    CHECK(bus.ReadCpu(0x15) == 0x81); // old Carry rotated into bit 7
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Negative set
}

TEST_CASE("RorZeroPageX wraps within the zero page instead of crossing into page 1") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xFF); // base operand
    cpu.SetXRegister(0x02);
    bus.WriteCpu(0x01, 0x06); // value at wrapped address (0xFF + 0x02) & 0xFF

    cpu.RorZeroPageX();

    CHECK(bus.ReadCpu(0x01) == 0x03);
}

TEST_CASE("RorAbsolute rotates the value at a 16-bit address and sets Zero and Carry") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, address = 0x0200
    bus.WriteCpu(0x0200, 0x01); // 0000 0001

    cpu.RorAbsolute();

    CHECK(bus.ReadCpu(0x0200) == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("RorAbsoluteX rotates the value at a 16-bit address plus X") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, base address = 0x0200
    cpu.SetXRegister(0x10);
    bus.WriteCpu(0x0210, 0x04);

    cpu.RorAbsoluteX();

    CHECK(bus.ReadCpu(0x0210) == 0x02);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}