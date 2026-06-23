#include "../doctest.h"

#include <stdexcept>

#include "../src/Bus.h"
#include "../src/Cpu.h"

namespace {

void ResetStatusRegister(nes::Cpu& cpu) {
    cpu.SetFlag(nes::Cpu::StatusFlag::C, false);
    cpu.SetFlag(nes::Cpu::StatusFlag::Z, false);
    cpu.SetFlag(nes::Cpu::StatusFlag::I, true);
    cpu.SetFlag(nes::Cpu::StatusFlag::D, false);
    cpu.SetFlag(nes::Cpu::StatusFlag::B, false);
    cpu.SetFlag(nes::Cpu::StatusFlag::U, true);
    cpu.SetFlag(nes::Cpu::StatusFlag::V, false);
    cpu.SetFlag(nes::Cpu::StatusFlag::N, false);
}

} // namespace

TEST_CASE("Cpu status string starts in the expected reset state") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    ResetStatusRegister(cpu);

    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("Cpu flag helpers update the status register") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    ResetStatusRegister(cpu);

    CHECK(cpu.StatusString() == "nvUbdIzc");

    cpu.SetZFlag(0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc");

    cpu.SetNFlag(0x80);
    CHECK(cpu.StatusString() == "NvUbdIZc");

    cpu.SetFlag(nes::Cpu::StatusFlag::Z, false);
    CHECK(cpu.StatusString() == "NvUbdIzc");
}

TEST_CASE("Cpu registers start in the documented power-up state") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    CHECK(cpu.GetXRegister() == 0x00);
    CHECK(cpu.GetStatusRegister() == 0x24);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("Inx increments the X register and updates the Zero and Negative flags") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Inx();
    CHECK(cpu.GetXRegister() == 0x01);
    CHECK(cpu.StatusString() == "nvUbdIzc"); // normal increment sets neither flag

    cpu.SetXRegister(0xFF);
    cpu.Inx(); // wraps from 0xFF to 0x00
    CHECK(cpu.GetXRegister() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc"); // Zero flag set

    cpu.SetXRegister(0x7F);
    cpu.Inx(); // 0x80, high bit set
    CHECK(cpu.GetXRegister() == 0x80);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Negative flag set
}

TEST_CASE("LdaImmediate updates the Zero and Negative flags based on the loaded value") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xA9); // LDA Immediate
    bus.WriteCpu(0x01, 0x00); // value = 0x00
    (void)cpu.Step();
    CHECK(cpu.StatusString() == "nvUbdIZc"); // Zero flag set, Negative clear

    bus.WriteCpu(0x02, 0xA9);
    bus.WriteCpu(0x03, 0x80); // value = 0x80
    (void)cpu.Step();
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Negative flag set, Zero clear

    bus.WriteCpu(0x04, 0xA9);
    bus.WriteCpu(0x05, 0x01); // value = 0x01
    (void)cpu.Step();
    CHECK(cpu.StatusString() == "nvUbdIzc"); // both flags clear
}

TEST_CASE("Step dispatches each implemented opcode and returns its cycle count") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xA9); // LDA Immediate
    bus.WriteCpu(0x01, 0x42);
    CHECK(cpu.Step() == 2);

    bus.WriteCpu(0x02, 0x85); // STA Zero Page
    bus.WriteCpu(0x03, 0x10);
    CHECK(cpu.Step() == 3);
    CHECK(bus.ReadCpu(0x10) == 0x42);

    bus.WriteCpu(0x04, 0x8D); // STA Absolute
    bus.WriteCpu(0x05, 0x00);
    bus.WriteCpu(0x06, 0x02);
    CHECK(cpu.Step() == 4);
    CHECK(bus.ReadCpu(0x0200) == 0x42);

    bus.WriteCpu(0x07, 0xE8); // INX
    CHECK(cpu.Step() == 2);
    CHECK(cpu.GetXRegister() == 0x01);
}

TEST_CASE("Step throws for an unimplemented opcode") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xFF); // not a recognized opcode

    CHECK_THROWS_AS((void)cpu.Step(), std::runtime_error);
}


