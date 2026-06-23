#include "../doctest.h"

#include "../src/Bus.h"
#include "../src/Cpu.h"

TEST_CASE("StackPushByte writes the value to the stack page and decrements the stack pointer") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.StackPushByte(0x42);

    CHECK(bus.ReadCpu(0x01FD) == 0x42); // stack pointer starts at 0xFD
}

TEST_CASE("StackPopByte reads the value back and increments the stack pointer") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.StackPushByte(0x42);
    CHECK(cpu.StackPopByte() == 0x42);
}

TEST_CASE("StackPushByte and StackPopByte round trip multiple values in LIFO order") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.StackPushByte(0x11);
    cpu.StackPushByte(0x22);
    cpu.StackPushByte(0x33);

    CHECK(cpu.StackPopByte() == 0x33);
    CHECK(cpu.StackPopByte() == 0x22);
    CHECK(cpu.StackPopByte() == 0x11);
}

TEST_CASE("StackPushWord writes the high byte then the low byte, decrementing the stack pointer twice") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.StackPushWord(0x1234);

    CHECK(bus.ReadCpu(0x01FD) == 0x12); // high byte pushed first
    CHECK(bus.ReadCpu(0x01FC) == 0x34); // low byte pushed second
}

TEST_CASE("StackPopWord reconstructs the 16-bit value pushed by StackPushWord") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.StackPushWord(0x1234);

    CHECK(cpu.StackPopWord() == 0x1234);
}

TEST_CASE("StackPushWord and StackPopWord round trip multiple values in LIFO order") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.StackPushWord(0xABCD);
    cpu.StackPushWord(0x0102);

    CHECK(cpu.StackPopWord() == 0x0102);
    CHECK(cpu.StackPopWord() == 0xABCD);
}

TEST_CASE("Stack pointer wraps from 0x00 to 0xFF when pushing past the bottom of the stack") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    // Drive the stack pointer down to 0x00 by pushing 0xFD bytes (starts at 0xFD).
    for (int i = 0; i < 0xFD; ++i) {
        cpu.StackPushByte(static_cast<std::uint8_t>(i));
    }

    cpu.StackPushByte(0x99); // stack pointer wraps from 0x00 to 0xFF

    CHECK(bus.ReadCpu(0x0100) == 0x99);
}

TEST_CASE("Stack pointer wraps from 0xFF to 0x00 when popping past the top of the stack") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x0100, 0x77); // value at the wrapped-to address

    cpu.StackPopByte(); // stack pointer 0xFD -> 0xFE
    cpu.StackPopByte(); // stack pointer 0xFE -> 0xFF

    CHECK(cpu.StackPopByte() == 0x77); // stack pointer wraps from 0xFF to 0x00
}
