#include "doctest.h"

#include "../src/core/Bus.h"
#include "TestBus.h"
#include "../src/core/cpu/Cpu.h"

TEST_CASE("Adc adds the value to the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x10);

    cpu.Adc(0x05);

    CHECK(cpu.GetAccumulator() == 0x15);
    CHECK(cpu.StatusString() == "nvUbdIzc"); // Carry, Zero, Negative, Overflow clear
}

TEST_CASE("Adc includes the Carry flag in the addition") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x10);
    cpu.SetCFlag(true);

    cpu.Adc(0x05);

    CHECK(cpu.GetAccumulator() == 0x16); // 0x10 + 0x05 + carry-in
}

TEST_CASE("Adc sets Carry when the unsigned result overflows 0xFF") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xFF);

    cpu.Adc(0x02);

    CHECK(cpu.GetAccumulator() == 0x01); // wraps around
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero clear, Negative clear
}

TEST_CASE("Adc sets Zero when the result is 0x00") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xFF);

    cpu.Adc(0x01);

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("Adc sets Negative when bit 7 of the result is set") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x10); // +16

    cpu.Adc(0x80); // -128, signs differ so no overflow

    CHECK(cpu.GetAccumulator() == 0x90);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Negative set, Carry and Zero clear
}

TEST_CASE("Adc sets Overflow when two positive operands produce a negative result") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x50); // +80

    cpu.Adc(0x50); // +80, sum is +160 which wraps into negative range

    CHECK(cpu.GetAccumulator() == 0xA0);
    CHECK(cpu.StatusString() == "NVUbdIzc"); // Overflow set, Negative set, Carry clear
}

TEST_CASE("Adc sets Overflow when two negative operands produce a positive result") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x90); // -112

    cpu.Adc(0x90); // -112, sum overflows into positive range

    CHECK(cpu.GetAccumulator() == 0x20);
    CHECK(cpu.StatusString() == "nVUbdIzC"); // Overflow set, Carry set, Negative clear
}

TEST_CASE("Adc does not set Overflow when operands have different signs") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x50); // +80

    cpu.Adc(0x90); // -112

    CHECK(cpu.GetAccumulator() == 0xE0);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Overflow clear, Negative set
}

TEST_CASE("AdcImmediate adds the value following the opcode to the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x10);
    bus.WriteCpu(0x00, 0x05);

    cpu.AdcImmediate();

    CHECK(cpu.GetProgramCounter() == 0x0001); // one operand byte consumed
    CHECK(cpu.GetAccumulator() == 0x15);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("AdcImmediate sets Carry and Zero when the result wraps to 0x00") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xFF);
    bus.WriteCpu(0x00, 0x01);

    cpu.AdcImmediate();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("AdcZeroPage adds the value at the zero page address to the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x10);
    bus.WriteCpu(0x00, 0x20); // zero page address
    bus.WriteCpu(0x20, 0x05); // value, result = 0x15

    cpu.AdcZeroPage();

    CHECK(cpu.GetProgramCounter() == 0x0001); // one operand byte consumed
    CHECK(cpu.GetAccumulator() == 0x15);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("AdcZeroPageX adds the value at the zero page address plus X to the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x10);
    bus.WriteCpu(0x00, 0x10); // base zero page address
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x20); // value at base+X, result = 0x30

    cpu.AdcZeroPageX();

    CHECK(cpu.GetProgramCounter() == 0x0001); // one operand byte consumed
    CHECK(cpu.GetAccumulator() == 0x30);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("AdcAbsolute adds the value at a 16-bit address to the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x10);
    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, address = 0x0200
    bus.WriteCpu(0x0200, 0x05); // value, result = 0x15

    cpu.AdcAbsolute();

    CHECK(cpu.GetProgramCounter() == 0x0002); // two operand bytes consumed
    CHECK(cpu.GetAccumulator() == 0x15);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("AdcAbsoluteX adds the value at a 16-bit address plus X to the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x10);
    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, base address = 0x0200
    cpu.SetXRegister(0x10);
    bus.WriteCpu(0x0210, 0x05); // value at base+X, result = 0x15

    cpu.AdcAbsoluteX();

    CHECK(cpu.GetProgramCounter() == 0x0002); // two operand bytes consumed
    CHECK(cpu.GetAccumulator() == 0x15);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("AdcAbsoluteY adds the value at a 16-bit address plus Y to the accumulator") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x10);
    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, base address = 0x0200
    cpu.SetYRegister(0x10);
    bus.WriteCpu(0x0210, 0x05); // value at base+Y, result = 0x15

    cpu.AdcAbsoluteY();

    CHECK(cpu.GetProgramCounter() == 0x0002); // two operand bytes consumed
    CHECK(cpu.GetAccumulator() == 0x15);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("AdcIndirectX adds X to the zero page base, reads a pointer, then adds the value") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x10);
    bus.WriteCpu(0x00, 0x10); // base zero page operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x78); // pointer low byte
    bus.WriteCpu(0x16, 0x06); // pointer high byte, target address = 0x0678
    bus.WriteCpu(0x0678, 0x05); // value, result = 0x15

    cpu.AdcIndirectX();

    CHECK(cpu.GetProgramCounter() == 0x0001); // one operand byte consumed
    CHECK(cpu.GetAccumulator() == 0x15);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("AdcIndirectY reads a zero page pointer, adds Y, then adds the value") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x10);
    bus.WriteCpu(0x00, 0x20); // zero page pointer location
    bus.WriteCpu(0x20, 0x00); // pointer low byte
    bus.WriteCpu(0x21, 0x02); // pointer high byte, base address = 0x0200
    cpu.SetYRegister(0x05);
    bus.WriteCpu(0x0205, 0x05); // value at base+Y, result = 0x15

    cpu.AdcIndirectY();

    CHECK(cpu.GetProgramCounter() == 0x0001); // one operand byte consumed
    CHECK(cpu.GetAccumulator() == 0x15);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}