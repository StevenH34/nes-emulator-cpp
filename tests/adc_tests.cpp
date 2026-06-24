#include "../doctest.h"

#include "../src/Bus.h"
#include "../src/Cpu.h"

TEST_CASE("Adc adds the value to the accumulator") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x10);

    cpu.Adc(0x05);

    CHECK(cpu.GetAccumulator() == 0x15);
    CHECK(cpu.StatusString() == "nvUbdIzc"); // Carry, Zero, Negative, Overflow clear
}

TEST_CASE("Adc includes the Carry flag in the addition") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x10);
    cpu.SetCFlag(true);

    cpu.Adc(0x05);

    CHECK(cpu.GetAccumulator() == 0x16); // 0x10 + 0x05 + carry-in
}

TEST_CASE("Adc sets Carry when the unsigned result overflows 0xFF") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xFF);

    cpu.Adc(0x02);

    CHECK(cpu.GetAccumulator() == 0x01); // wraps around
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero clear, Negative clear
}

TEST_CASE("Adc sets Zero when the result is 0x00") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xFF);

    cpu.Adc(0x01);

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("Adc sets Negative when bit 7 of the result is set") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x10); // +16

    cpu.Adc(0x80); // -128, signs differ so no overflow

    CHECK(cpu.GetAccumulator() == 0x90);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Negative set, Carry and Zero clear
}

TEST_CASE("Adc sets Overflow when two positive operands produce a negative result") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x50); // +80

    cpu.Adc(0x50); // +80, sum is +160 which wraps into negative range

    CHECK(cpu.GetAccumulator() == 0xA0);
    CHECK(cpu.StatusString() == "NVUbdIzc"); // Overflow set, Negative set, Carry clear
}

TEST_CASE("Adc sets Overflow when two negative operands produce a positive result") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x90); // -112

    cpu.Adc(0x90); // -112, sum overflows into positive range

    CHECK(cpu.GetAccumulator() == 0x20);
    CHECK(cpu.StatusString() == "nVUbdIzC"); // Overflow set, Carry set, Negative clear
}

TEST_CASE("Adc does not set Overflow when operands have different signs") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x50); // +80

    cpu.Adc(0x90); // -112

    CHECK(cpu.GetAccumulator() == 0xE0);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Overflow clear, Negative set
}

TEST_CASE("AdcImmediate adds the value following the opcode to the accumulator") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x10);
    bus.WriteCpu(0x00, 0x05);

    cpu.AdcImmediate();

    CHECK(cpu.GetProgramCounter() == 0x0001); // one operand byte consumed
    CHECK(cpu.GetAccumulator() == 0x15);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("AdcImmediate sets Carry and Zero when the result wraps to 0x00") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xFF);
    bus.WriteCpu(0x00, 0x01);

    cpu.AdcImmediate();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}