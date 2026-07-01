#include "../doctest.h"

#include "../src/Bus.h"
#include "../src/Cpu.h"

TEST_CASE("Nop leaves registers, flags, and the program counter unchanged") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x42);
    cpu.SetXRegister(0x11);
    cpu.SetYRegister(0x22);

    const auto status_before = cpu.GetStatusRegister();
    const auto pc_before = cpu.GetProgramCounter();

    cpu.Nop();

    CHECK(cpu.GetAccumulator() == 0x42);
    CHECK(cpu.GetXRegister() == 0x11);
    CHECK(cpu.GetYRegister() == 0x22);
    CHECK(cpu.GetStatusRegister() == status_before);
    CHECK(cpu.GetProgramCounter() == pc_before);
}