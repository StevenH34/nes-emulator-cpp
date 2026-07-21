#include "doctest.h"

#include "../../../src/core/Bus.h"
#include "TestBus.h"
#include "../../../src/core/cpu/Cpu.h"

TEST_CASE("StackPushByte writes the value to the stack page and decrements the stack pointer") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.StackPushByte(0x42);

    CHECK(bus.ReadCpu(0x01FD) == 0x42); // stack pointer starts at 0xFD
}

TEST_CASE("StackPullByte reads the value back and increments the stack pointer") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.StackPushByte(0x42);
    CHECK(cpu.StackPullByte() == 0x42);
}

TEST_CASE("StackPushByte and StackPullByte round trip multiple values in LIFO order") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.StackPushByte(0x11);
    cpu.StackPushByte(0x22);
    cpu.StackPushByte(0x33);

    CHECK(cpu.StackPullByte() == 0x33);
    CHECK(cpu.StackPullByte() == 0x22);
    CHECK(cpu.StackPullByte() == 0x11);
}

TEST_CASE("StackPushWord writes the high byte then the low byte, decrementing the stack pointer twice") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.StackPushWord(0x1234);

    CHECK(bus.ReadCpu(0x01FD) == 0x12); // high byte pushed first
    CHECK(bus.ReadCpu(0x01FC) == 0x34); // low byte pushed second
}

TEST_CASE("StackPullWord reconstructs the 16-bit value pushed by StackPushWord") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.StackPushWord(0x1234);

    CHECK(cpu.StackPullWord() == 0x1234);
}

TEST_CASE("StackPushWord and StackPullWord round trip multiple values in LIFO order") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.StackPushWord(0xABCD);
    cpu.StackPushWord(0x0102);

    CHECK(cpu.StackPullWord() == 0x0102);
    CHECK(cpu.StackPullWord() == 0xABCD);
}

TEST_CASE("Stack pointer wraps from 0x00 to 0xFF when pushing past the bottom of the stack") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    // Drive the stack pointer down to 0x00 by pushing 0xFD bytes (starts at 0xFD).
    for (int i = 0; i < 0xFD; ++i) {
        cpu.StackPushByte(static_cast<std::uint8_t>(i));
    }

    cpu.StackPushByte(0x99); // stack pointer wraps from 0x00 to 0xFF

    CHECK(bus.ReadCpu(0x0100) == 0x99);
}

TEST_CASE("Stack pointer wraps from 0xFF to 0x00 when popping past the top of the stack") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x0100, 0x77); // value at the wrapped-to address

    cpu.StackPullByte(); // stack pointer 0xFD -> 0xFE
    cpu.StackPullByte(); // stack pointer 0xFE -> 0xFF

    CHECK(cpu.StackPullByte() == 0x77); // stack pointer wraps from 0xFF to 0x00
}

TEST_CASE("Pha pushes the accumulator onto the stack and decrements the stack pointer") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xA9); // LDA Immediate
    bus.WriteCpu(0x01, 0x42);
    CHECK(cpu.Step() == 2);

    bus.WriteCpu(0x02, 0x48); // PHA
    CHECK(cpu.Step() == 3);
    CHECK(bus.ReadCpu(0x01FD) == 0x42); // written at the pre-push stack pointer location

    bus.WriteCpu(0x03, 0xBA); // TSX - confirms the stack pointer decremented
    CHECK(cpu.Step() == 2);
    CHECK(cpu.GetXRegister() == 0xFC);
}

TEST_CASE("Pla pops a value from the stack into the accumulator and updates the Negative flag") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xA9); // LDA #$80
    bus.WriteCpu(0x01, 0x80);
    CHECK(cpu.Step() == 2);

    bus.WriteCpu(0x02, 0x48); // PHA
    CHECK(cpu.Step() == 3);

    bus.WriteCpu(0x03, 0xA9); // LDA #$00, clear the accumulator
    bus.WriteCpu(0x04, 0x00);
    CHECK(cpu.Step() == 2);
    CHECK(cpu.GetAccumulator() == 0x00);

    bus.WriteCpu(0x05, 0x68); // PLA
    CHECK(cpu.Step() == 4);
    CHECK(cpu.GetAccumulator() == 0x80);
    CHECK(cpu.IsFlagSet(static_cast<std::uint8_t>(nes::Cpu::StatusFlag::N)));
    CHECK_FALSE(cpu.IsFlagSet(static_cast<std::uint8_t>(nes::Cpu::StatusFlag::Z)));

    bus.WriteCpu(0x06, 0xBA); // TSX - confirms the stack pointer was restored
    CHECK(cpu.Step() == 2);
    CHECK(cpu.GetXRegister() == 0xFD);
}

TEST_CASE("Pla sets the Zero flag when popping a zero value") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xA9); // LDA #$00
    bus.WriteCpu(0x01, 0x00);
    CHECK(cpu.Step() == 2);

    bus.WriteCpu(0x02, 0x48); // PHA
    CHECK(cpu.Step() == 3);

    bus.WriteCpu(0x03, 0xA9); // LDA #$FF, dirty the accumulator before popping
    bus.WriteCpu(0x04, 0xFF);
    CHECK(cpu.Step() == 2);

    bus.WriteCpu(0x05, 0x68); // PLA
    CHECK(cpu.Step() == 4);
    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.IsFlagSet(static_cast<std::uint8_t>(nes::Cpu::StatusFlag::Z)));
    CHECK_FALSE(cpu.IsFlagSet(static_cast<std::uint8_t>(nes::Cpu::StatusFlag::N)));
}

TEST_CASE("Php pushes the status register with the Break and Unused flags forced on") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.SetFlag(nes::Cpu::StatusFlag::C, true);
    cpu.SetFlag(nes::Cpu::StatusFlag::N, true);
    cpu.SetFlag(nes::Cpu::StatusFlag::I, false);
    const auto status_before = cpu.GetStatusRegister();

    bus.WriteCpu(0x00, 0x08); // PHP
    CHECK(cpu.Step() == 3);

    const std::uint8_t expected = status_before
        | static_cast<std::uint8_t>(nes::Cpu::StatusFlag::B)
        | static_cast<std::uint8_t>(nes::Cpu::StatusFlag::U);
    CHECK(bus.ReadCpu(0x01FD) == expected);
    CHECK(cpu.GetStatusRegister() == status_before); // Php does not modify the live status register

    bus.WriteCpu(0x01, 0xBA); // TSX - confirms the stack pointer decremented
    CHECK(cpu.Step() == 2);
    CHECK(cpu.GetXRegister() == 0xFC);
}

TEST_CASE("Plp restores flags from the stack while forcing Unused on and Break off") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.SetFlag(nes::Cpu::StatusFlag::C, true);
    cpu.SetFlag(nes::Cpu::StatusFlag::N, true);
    cpu.SetFlag(nes::Cpu::StatusFlag::V, true);

    bus.WriteCpu(0x00, 0x08); // PHP
    CHECK(cpu.Step() == 3);

    // Change flags after pushing so we can confirm Plp restores the pushed values
    cpu.SetFlag(nes::Cpu::StatusFlag::C, false);
    cpu.SetFlag(nes::Cpu::StatusFlag::N, false);
    cpu.SetFlag(nes::Cpu::StatusFlag::V, false);

    bus.WriteCpu(0x01, 0x28); // PLP
    CHECK(cpu.Step() == 4);

    CHECK(cpu.IsFlagSet(static_cast<std::uint8_t>(nes::Cpu::StatusFlag::C)));
    CHECK(cpu.IsFlagSet(static_cast<std::uint8_t>(nes::Cpu::StatusFlag::N)));
    CHECK(cpu.IsFlagSet(static_cast<std::uint8_t>(nes::Cpu::StatusFlag::V)));
    CHECK(cpu.IsFlagSet(static_cast<std::uint8_t>(nes::Cpu::StatusFlag::U))); // Always forced on
    CHECK_FALSE(cpu.IsFlagSet(static_cast<std::uint8_t>(nes::Cpu::StatusFlag::B))); // Always forced off

    bus.WriteCpu(0x02, 0xBA); // TSX - confirms the stack pointer was restored
    CHECK(cpu.Step() == 2);
    CHECK(cpu.GetXRegister() == 0xFD);
}
