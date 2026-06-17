#include "../doctest.h"

#include "../Bus.h"
#include "../Cpu.h"

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
    (void)cpu;
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

    (void)cpu;
}




