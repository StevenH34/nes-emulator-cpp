#include "doctest.h"

#include "../src/Bus.h"
#include "TestBus.h"
#include "../src/cpu/Cpu.h"

TEST_CASE("Beq branches when the Zero flag is set") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.SetFlag(nes::Cpu::StatusFlag::Z, true);
    bus.WriteCpu(0x00, 0x05); // offset = +5

    cpu.Beq();

    CHECK(cpu.GetProgramCounter() == 0x0006); // PC 0x0001 (after fetch) + 5
}

TEST_CASE("Beq does not branch but still consumes the offset byte when the Zero flag is clear") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.SetFlag(nes::Cpu::StatusFlag::Z, false);
    bus.WriteCpu(0x00, 0x05); // offset = +5, should be ignored

    cpu.Beq();

    CHECK(cpu.GetProgramCounter() == 0x0001); // only the offset byte was consumed
}

TEST_CASE("Bne branches when the Zero flag is clear") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.SetFlag(nes::Cpu::StatusFlag::Z, false);
    bus.WriteCpu(0x00, 0x05);

    cpu.Bne();

    CHECK(cpu.GetProgramCounter() == 0x0006);
}

TEST_CASE("Bne does not branch when the Zero flag is set") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.SetFlag(nes::Cpu::StatusFlag::Z, true);
    bus.WriteCpu(0x00, 0x05);

    cpu.Bne();

    CHECK(cpu.GetProgramCounter() == 0x0001);
}

TEST_CASE("Bcs branches when the Carry flag is set") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.SetFlag(nes::Cpu::StatusFlag::C, true);
    bus.WriteCpu(0x00, 0x10);

    cpu.Bcs();

    CHECK(cpu.GetProgramCounter() == 0x0011);
}

TEST_CASE("Bcc branches when the Carry flag is clear") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.SetFlag(nes::Cpu::StatusFlag::C, false);
    bus.WriteCpu(0x00, 0x10);

    cpu.Bcc();

    CHECK(cpu.GetProgramCounter() == 0x0011);
}

TEST_CASE("Bcc does not branch when the Carry flag is set") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.SetFlag(nes::Cpu::StatusFlag::C, true);
    bus.WriteCpu(0x00, 0x10);

    cpu.Bcc();

    CHECK(cpu.GetProgramCounter() == 0x0001);
}

TEST_CASE("Bmi branches when the Negative flag is set") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.SetFlag(nes::Cpu::StatusFlag::N, true);
    bus.WriteCpu(0x00, 0x20);

    cpu.Bmi();

    CHECK(cpu.GetProgramCounter() == 0x0021);
}

TEST_CASE("Bpl branches when the Negative flag is clear") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.SetFlag(nes::Cpu::StatusFlag::N, false);
    bus.WriteCpu(0x00, 0x20);

    cpu.Bpl();

    CHECK(cpu.GetProgramCounter() == 0x0021);
}

TEST_CASE("Bpl does not branch when the Negative flag is set") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.SetFlag(nes::Cpu::StatusFlag::N, true);
    bus.WriteCpu(0x00, 0x20);

    cpu.Bpl();

    CHECK(cpu.GetProgramCounter() == 0x0001);
}

TEST_CASE("Bvs branches when the Overflow flag is set") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.SetFlag(nes::Cpu::StatusFlag::V, true);
    bus.WriteCpu(0x00, 0x30);

    cpu.Bvs();

    CHECK(cpu.GetProgramCounter() == 0x0031);
}

TEST_CASE("Bvc branches when the Overflow flag is clear") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.SetFlag(nes::Cpu::StatusFlag::V, false);
    bus.WriteCpu(0x00, 0x30);

    cpu.Bvc();

    CHECK(cpu.GetProgramCounter() == 0x0031);
}

TEST_CASE("Bvc does not branch when the Overflow flag is set") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.SetFlag(nes::Cpu::StatusFlag::V, true);
    bus.WriteCpu(0x00, 0x30);

    cpu.Bvc();

    CHECK(cpu.GetProgramCounter() == 0x0001);
}

TEST_CASE("BranchIf supports backward branches with a negative offset") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xA9); // LDA Immediate
    bus.WriteCpu(0x01, 0x10); // LDA value, advances PC to 0x0002
    CHECK(cpu.Step() == 2);

    bus.WriteCpu(0x02, 0xFE); // offset = -2
    cpu.SetFlag(nes::Cpu::StatusFlag::Z, true);

    cpu.Beq();

    CHECK(cpu.GetProgramCounter() == 0x0001); // PC 0x0003 (after fetch) - 2
}

TEST_CASE("JmpAbsolute sets the Program Counter to the 16-bit operand address") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x34); // low byte
    bus.WriteCpu(0x01, 0x12); // high byte

    cpu.JmpAbsolute();

    CHECK(cpu.GetProgramCounter() == 0x1234);
}

TEST_CASE("JmpIndirect sets the Program Counter to the address stored at the pointer") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // pointer low byte
    bus.WriteCpu(0x01, 0x03); // pointer high byte, pointer address = 0x0300
    bus.WriteCpu(0x0300, 0x34); // target low byte
    bus.WriteCpu(0x0301, 0x12); // target high byte

    cpu.JmpIndirect();

    CHECK(cpu.GetProgramCounter() == 0x1234);
}

TEST_CASE("JmpIndirect reproduces the 6502 page-boundary bug") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xFF); // pointer low byte
    bus.WriteCpu(0x01, 0x02); // pointer high byte, pointer address = 0x02FF
    bus.WriteCpu(0x02FF, 0x80); // target low byte
    bus.WriteCpu(0x0200, 0x01); // target high byte read from the wrapped page start
    bus.WriteCpu(0x0300, 0xFF); // would be (wrongly) read if the page didn't wrap

    cpu.JmpIndirect();

    CHECK(cpu.GetProgramCounter() == 0x0180);
}

TEST_CASE("Jsr pushes the return address and sets the Program Counter to the target") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x34); // low byte
    bus.WriteCpu(0x01, 0x12); // high byte

    cpu.Jsr();

    CHECK(cpu.GetProgramCounter() == 0x1234);
    CHECK(bus.ReadCpu(0x01FD) == 0x00); // high byte of PC - 1 (0x0001)
    CHECK(bus.ReadCpu(0x01FC) == 0x01); // low byte of PC - 1 (0x0001)
}

TEST_CASE("Rts pops the return address from the stack and sets the Program Counter to address + 1") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.StackPushWord(0x1233); // return address - 1, as Jsr would have pushed it

    cpu.Rts();

    CHECK(cpu.GetProgramCounter() == 0x1234);
}

TEST_CASE("Jsr followed by Rts returns to the instruction after the JSR operand") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x34); // low byte of subroutine address
    bus.WriteCpu(0x01, 0x12); // high byte of subroutine address

    cpu.Jsr();
    CHECK(cpu.GetProgramCounter() == 0x1234);

    cpu.Rts();
    CHECK(cpu.GetProgramCounter() == 0x0002); // back to the instruction after JSR's operand
}

TEST_CASE("Brk pushes the Program Counter and Status Register, sets the Interrupt flag, and jumps to the IRQ vector address") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.SetFlag(nes::Cpu::StatusFlag::I, false);

    cpu.Brk();

    CHECK(bus.ReadCpu(0x01FD) == 0x00); // high byte of PC + 1 (0x0001)
    CHECK(bus.ReadCpu(0x01FC) == 0x01); // low byte of PC + 1 (0x0001)
    CHECK(bus.ReadCpu(0x01FB) ==
        (static_cast<std::uint8_t>(nes::Cpu::StatusFlag::U) | static_cast<std::uint8_t>(nes::Cpu::StatusFlag::B))); // pushed status has Break and Unused set
    CHECK(cpu.IsFlagSet(static_cast<std::uint8_t>(nes::Cpu::StatusFlag::I))); // Interrupt Disable flag set after the push
    CHECK(cpu.GetProgramCounter() == 0x0000); // IRQ vector ($FFFE/$FFFF) is unmapped and reads as 0
}

TEST_CASE("Rti restores the Program Counter and Status Register from the stack, clearing the Break flag") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.StackPushWord(0x1234); // Program Counter, as Brk would have pushed it
    cpu.StackPushByte(0xFF); // Status Register, as Brk would have pushed it (all flags set, including Break)

    cpu.Rti();

    CHECK(cpu.GetProgramCounter() == 0x1234);
    CHECK(cpu.GetStatusRegister() == (0xFF & ~static_cast<std::uint8_t>(nes::Cpu::StatusFlag::B))); // Break cleared, Unused stays on
}

TEST_CASE("Rti forces the Unused flag on even if it was not set on the stack") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.StackPushWord(0x0000);
    cpu.StackPushByte(0x00); // no flags set on the stack

    cpu.Rti();

    CHECK(cpu.GetStatusRegister() == static_cast<std::uint8_t>(nes::Cpu::StatusFlag::U));
}

TEST_CASE("Brk followed by Rti returns to the Program Counter and Status Register from before the interrupt") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xA9); // LDA Immediate
    bus.WriteCpu(0x01, 0x42); // LDA value, advances PC to 0x0002
    CHECK(cpu.Step() == 2);

    const std::uint8_t status_before = cpu.GetStatusRegister();
    const std::uint16_t pc_before = cpu.GetProgramCounter();

    cpu.Brk();
    cpu.Rti();

    CHECK(cpu.GetProgramCounter() == pc_before + 1); // Brk pushed PC + 1, Rti restores it unchanged
    CHECK(cpu.GetStatusRegister() == status_before); // Break never persists, Unused was already set
}
