#include "doctest.h"

#include "../src/Bus.h"
#include "../src/Cpu.h"

TEST_CASE("BitZeroPage sets Zero when the AND of the accumulator and memory value is zero, without modifying the accumulator") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x0F);
    bus.WriteCpu(0x00, 0x10); // zero page address operand
    bus.WriteCpu(0x10, 0xF0); // value; A & M = 0x00

    cpu.BitZeroPage();

    CHECK(cpu.GetAccumulator() == 0x0F); // accumulator is unchanged by BIT
    CHECK(cpu.StatusString() == "NVUbdIZc"); // Zero set from AND result, Negative/Overflow set from bits 7/6 of value
}

TEST_CASE("BitZeroPage clears Zero when the AND of the accumulator and memory value is nonzero") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xFF);
    bus.WriteCpu(0x00, 0x10); // zero page address operand
    bus.WriteCpu(0x10, 0x01); // value; A & M = 0x01, bits 7 and 6 clear

    cpu.BitZeroPage();

    CHECK(cpu.StatusString() == "nvUbdIzc"); // Zero clear, Negative/Overflow clear
}

TEST_CASE("BitZeroPage sets Negative and Overflow from bits 7 and 6 of the memory value, not from the AND result") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xC0);
    bus.WriteCpu(0x00, 0x10); // zero page address operand
    bus.WriteCpu(0x10, 0xC1); // value; A & M = 0xC0 (nonzero), but bits 7 and 6 of the value are set

    cpu.BitZeroPage();

    CHECK(cpu.GetAccumulator() == 0xC0); // accumulator is unchanged
    CHECK(cpu.StatusString() == "NVUbdIzc"); // Zero clear (AND nonzero), Negative/Overflow set from the raw value
}

TEST_CASE("BitAbsolute sets Zero, Negative, and Overflow from the value at a 16-bit address, without modifying the accumulator") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x0F);
    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, address = 0x0200
    bus.WriteCpu(0x0200, 0xF0); // value; A & M = 0x00

    cpu.BitAbsolute();

    CHECK(cpu.GetAccumulator() == 0x0F); // accumulator is unchanged
    CHECK(cpu.StatusString() == "NVUbdIZc"); // Zero set from AND result, Negative/Overflow set from bits 7/6 of value
}

TEST_CASE("BitAbsolute clears Zero when the AND of the accumulator and memory value is nonzero") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0xFF);
    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, address = 0x0200
    bus.WriteCpu(0x0200, 0x01); // value; A & M = 0x01, bits 7 and 6 clear

    cpu.BitAbsolute();

    CHECK(cpu.StatusString() == "nvUbdIzc"); // Zero clear, Negative/Overflow clear
}