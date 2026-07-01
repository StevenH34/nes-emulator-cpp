#include "../doctest.h"

#include "../src/Bus.h"
#include "../src/Cpu.h"

TEST_CASE("AslAccumulator shifts bits left and clears Carry, Zero, and Negative") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x01); // 0000 0001

    cpu.AslAccumulator();

    CHECK(cpu.GetAccumulator() == 0x02); // 0000 0010
    CHECK(cpu.StatusString() == "nvUbdIzc"); // Carry clear, Zero clear, Negative clear
}

TEST_CASE("AslAccumulator sets Carry when bit 7 is shifted out") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x81); // 1000 0001

    cpu.AslAccumulator();

    CHECK(cpu.GetAccumulator() == 0x02); // 0000 0010, bit 7 dropped
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero clear, Negative clear
}

TEST_CASE("AslAccumulator sets Zero when the result is 0x00") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x80); // 1000 0000

    cpu.AslAccumulator();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("AslAccumulator sets Negative when bit 7 of the result is set") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x40); // 0100 0000

    cpu.AslAccumulator();

    CHECK(cpu.GetAccumulator() == 0x80); // 1000 0000
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Carry clear, Zero clear, Negative set
}

TEST_CASE("Asl returns the shifted value without touching memory or the accumulator") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    CHECK(cpu.Asl(0x81) == 0x02); // 1000 0001 -> 0000 0010, bit 7 dropped
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero clear, Negative clear
    CHECK(cpu.GetAccumulator() == 0x00);
}

TEST_CASE("AslZeroPage shifts the value at the zero page address and stores the result") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // zero page address operand
    bus.WriteCpu(0x10, 0x01); // value at the zero page address

    cpu.AslZeroPage();

    CHECK(bus.ReadCpu(0x10) == 0x02);
    CHECK(cpu.StatusString() == "nvUbdIzc"); // Carry clear, Zero clear, Negative clear
}

TEST_CASE("AslZeroPage sets Carry when bit 7 is shifted out") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // zero page address operand
    bus.WriteCpu(0x10, 0x81); // 1000 0001

    cpu.AslZeroPage();

    CHECK(bus.ReadCpu(0x10) == 0x02); // bit 7 dropped
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero clear, Negative clear
}

TEST_CASE("AslZeroPageX shifts the value at the zero page address plus X") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x40); // value at 0x10 + X

    cpu.AslZeroPageX();

    CHECK(bus.ReadCpu(0x15) == 0x80);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Negative set
}

TEST_CASE("AslZeroPageX wraps within the zero page instead of crossing into page 1") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xFF); // base operand
    cpu.SetXRegister(0x02);
    bus.WriteCpu(0x01, 0x03); // value at wrapped address (0xFF + 0x02) & 0xFF

    cpu.AslZeroPageX();

    CHECK(bus.ReadCpu(0x01) == 0x06);
}

TEST_CASE("AslAbsolute shifts the value at a 16-bit address and sets Zero and Carry") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, address = 0x0200
    bus.WriteCpu(0x0200, 0x80); // 1000 0000

    cpu.AslAbsolute();

    CHECK(bus.ReadCpu(0x0200) == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("AslAbsoluteX shifts the value at a 16-bit address plus X") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, base address = 0x0200
    cpu.SetXRegister(0x10);
    bus.WriteCpu(0x0210, 0x01);

    cpu.AslAbsoluteX();

    CHECK(bus.ReadCpu(0x0210) == 0x02);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("LsrAccumulator shifts bits right and clears Carry and Zero") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x02); // 0000 0010

    cpu.LsrAccumulator();

    CHECK(cpu.GetAccumulator() == 0x01); // 0000 0001
    CHECK(cpu.StatusString() == "nvUbdIzc"); // Carry clear, Zero clear, Negative clear
}

TEST_CASE("LsrAccumulator sets Carry when bit 0 is shifted out") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x03); // 0000 0011

    cpu.LsrAccumulator();

    CHECK(cpu.GetAccumulator() == 0x01); // 0000 0001, bit 0 dropped
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero clear, Negative clear
}

TEST_CASE("LsrAccumulator sets Zero when the result is 0x00") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x01); // 0000 0001

    cpu.LsrAccumulator();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("LsrAccumulator always clears Negative because bit 7 becomes 0") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x81); // 1000 0001

    cpu.LsrAccumulator();

    CHECK(cpu.GetAccumulator() == 0x40); // 0100 0000
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set (bit 0 dropped), Negative clear
}

TEST_CASE("Lsr returns the shifted value without touching memory or the accumulator") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    CHECK(cpu.Lsr(0x03) == 0x01); // 0000 0011 -> 0000 0001, bit 0 dropped
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero clear, Negative clear
    CHECK(cpu.GetAccumulator() == 0x00);
}

TEST_CASE("LsrZeroPage shifts the value at the zero page address and stores the result") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // zero page address operand
    bus.WriteCpu(0x10, 0x02); // value at the zero page address

    cpu.LsrZeroPage();

    CHECK(bus.ReadCpu(0x10) == 0x01);
    CHECK(cpu.StatusString() == "nvUbdIzc"); // Carry clear, Zero clear, Negative clear
}

TEST_CASE("LsrZeroPage sets Carry when bit 0 is shifted out") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // zero page address operand
    bus.WriteCpu(0x10, 0x01); // 0000 0001

    cpu.LsrZeroPage();

    CHECK(bus.ReadCpu(0x10) == 0x00); // bit 0 dropped
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("LsrZeroPageX shifts the value at the zero page address plus X") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x80); // value at 0x10 + X

    cpu.LsrZeroPageX();

    CHECK(bus.ReadCpu(0x15) == 0x40);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("LsrZeroPageX wraps within the zero page instead of crossing into page 1") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xFF); // base operand
    cpu.SetXRegister(0x02);
    bus.WriteCpu(0x01, 0x06); // value at wrapped address (0xFF + 0x02) & 0xFF

    cpu.LsrZeroPageX();

    CHECK(bus.ReadCpu(0x01) == 0x03);
}

TEST_CASE("LsrAbsolute shifts the value at a 16-bit address and sets Zero and Carry") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, address = 0x0200
    bus.WriteCpu(0x0200, 0x01); // 0000 0001

    cpu.LsrAbsolute();

    CHECK(bus.ReadCpu(0x0200) == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("LsrAbsoluteX shifts the value at a 16-bit address plus X") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, base address = 0x0200
    cpu.SetXRegister(0x10);
    bus.WriteCpu(0x0210, 0x04);

    cpu.LsrAbsoluteX();

    CHECK(bus.ReadCpu(0x0210) == 0x02);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("RolAccumulator rotates bits left and shifts in a clear Carry") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x01); // 0000 0001

    cpu.RolAccumulator();

    CHECK(cpu.GetAccumulator() == 0x02); // 0000 0010
    CHECK(cpu.StatusString() == "nvUbdIzc"); // Carry clear, Zero clear, Negative clear
}

TEST_CASE("RolAccumulator shifts in a set Carry as bit 0") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x01); // 0000 0001
    cpu.SetCFlag(true);

    cpu.RolAccumulator();

    CHECK(cpu.GetAccumulator() == 0x03); // 0000 0011, old Carry becomes bit 0
    CHECK(cpu.StatusString() == "nvUbdIzc"); // Carry clear, Zero clear, Negative clear
}

TEST_CASE("RolAccumulator sets Carry when bit 7 is rotated out") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x81); // 1000 0001

    cpu.RolAccumulator();

    CHECK(cpu.GetAccumulator() == 0x02); // 0000 0010, bit 7 moved to Carry
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero clear, Negative clear
}

TEST_CASE("RolAccumulator sets Zero when the result is 0x00") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x80); // 1000 0000

    cpu.RolAccumulator();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("RolAccumulator sets Negative when bit 7 of the result is set") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x40); // 0100 0000

    cpu.RolAccumulator();

    CHECK(cpu.GetAccumulator() == 0x80); // 1000 0000
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Carry clear, Zero clear, Negative set
}

TEST_CASE("Rol returns the rotated value without touching memory or the accumulator") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    CHECK(cpu.Rol(0x81) == 0x02); // 1000 0001 -> 0000 0010, bit 7 moved to Carry
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero clear, Negative clear
    CHECK(cpu.GetAccumulator() == 0x00);
}

TEST_CASE("RolZeroPage rotates the value at the zero page address and stores the result") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // zero page address operand
    bus.WriteCpu(0x10, 0x01); // value at the zero page address

    cpu.RolZeroPage();

    CHECK(bus.ReadCpu(0x10) == 0x02);
    CHECK(cpu.StatusString() == "nvUbdIzc"); // Carry clear, Zero clear, Negative clear
}

TEST_CASE("RolZeroPage sets Carry when bit 7 is rotated out") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // zero page address operand
    bus.WriteCpu(0x10, 0x81); // 1000 0001

    cpu.RolZeroPage();

    CHECK(bus.ReadCpu(0x10) == 0x02); // bit 7 moved to Carry
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero clear, Negative clear
}

TEST_CASE("RolZeroPageX rotates the value at the zero page address plus X") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x40); // value at 0x10 + X

    cpu.RolZeroPageX();

    CHECK(bus.ReadCpu(0x15) == 0x80);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Negative set
}

TEST_CASE("RolZeroPageX wraps within the zero page instead of crossing into page 1") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xFF); // base operand
    cpu.SetXRegister(0x02);
    bus.WriteCpu(0x01, 0x03); // value at wrapped address (0xFF + 0x02) & 0xFF

    cpu.RolZeroPageX();

    CHECK(bus.ReadCpu(0x01) == 0x06);
}

TEST_CASE("RolAbsolute rotates the value at a 16-bit address and sets Zero and Carry") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, address = 0x0200
    bus.WriteCpu(0x0200, 0x80); // 1000 0000

    cpu.RolAbsolute();

    CHECK(bus.ReadCpu(0x0200) == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("RolAbsoluteX rotates the value at a 16-bit address plus X") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, base address = 0x0200
    cpu.SetXRegister(0x10);
    bus.WriteCpu(0x0210, 0x01);

    cpu.RolAbsoluteX();

    CHECK(bus.ReadCpu(0x0210) == 0x02);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}