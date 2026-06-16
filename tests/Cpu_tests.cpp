#include "../doctest.h"

#include "../Bus.h"
#include "../Cpu.h"

namespace {

void ResetStatusRegister() {
    nes::Cpu::SetFlag(0x01, false);
    nes::Cpu::SetFlag(0x02, false);
    nes::Cpu::SetFlag(0x04, true);
    nes::Cpu::SetFlag(0x08, false);
    nes::Cpu::SetFlag(0x10, false);
    nes::Cpu::SetFlag(0x20, true);
    nes::Cpu::SetFlag(0x40, false);
    nes::Cpu::SetFlag(0x80, false);
}

} // namespace

TEST_CASE("Cpu status string starts in the expected reset state") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    ResetStatusRegister();

    CHECK(nes::Cpu::StatusString() == "nvUbdIzc");
    (void)cpu;
}

TEST_CASE("Cpu flag helpers update the status register") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    ResetStatusRegister();

    CHECK(nes::Cpu::StatusString() == "nvUbdIzc");

    nes::Cpu::SetZFlag(0x00);
    CHECK(nes::Cpu::StatusString() == "nvUbdIZc");

    nes::Cpu::SetNFlag(0x80);
    CHECK(nes::Cpu::StatusString() == "NvUbdIZc");

    nes::Cpu::SetFlag(0x02, false);
    CHECK(nes::Cpu::StatusString() == "NvUbdIzc");

    (void)cpu;
}




