#include "doctest.h"

#include <stdexcept>

#include "../src/Bus.h"
#include "../src/Cartridge.h"
#include "TestBus.h"
#include "TestRom.h"
#include "../src/cpu/Cpu.h"

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
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    ResetStatusRegister(cpu);

    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("Cpu flag helpers update the status register") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    ResetStatusRegister(cpu);

    CHECK(cpu.StatusString() == "nvUbdIzc");

    cpu.SetZFlag(0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc");

    cpu.SetNFlag(0x80);
    CHECK(cpu.StatusString() == "NvUbdIZc");

    cpu.SetFlag(nes::Cpu::StatusFlag::Z, false);
    CHECK(cpu.StatusString() == "NvUbdIzc");

    cpu.SetZFlag(0x01); // non-zero clears the Zero flag
    CHECK(cpu.StatusString() == "NvUbdIzc");

    cpu.SetNFlag(0x01); // most significant bit clear clears the Negative flag
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("IsFlagSet reports whether a given status bit is set") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    ResetStatusRegister(cpu);

    CHECK_FALSE(cpu.IsFlagSet(static_cast<std::uint8_t>(nes::Cpu::StatusFlag::C)));
    CHECK_FALSE(cpu.IsFlagSet(static_cast<std::uint8_t>(nes::Cpu::StatusFlag::V)));

    cpu.SetFlag(nes::Cpu::StatusFlag::C, true);
    CHECK(cpu.IsFlagSet(static_cast<std::uint8_t>(nes::Cpu::StatusFlag::C)));
    CHECK_FALSE(cpu.IsFlagSet(static_cast<std::uint8_t>(nes::Cpu::StatusFlag::V)));

    cpu.SetFlag(nes::Cpu::StatusFlag::V, true);
    CHECK(cpu.IsFlagSet(static_cast<std::uint8_t>(nes::Cpu::StatusFlag::V)));
}

TEST_CASE("SetCFlag updates only the Carry flag") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    ResetStatusRegister(cpu);
    CHECK(cpu.StatusString() == "nvUbdIzc");

    cpu.SetCFlag(true);
    CHECK(cpu.StatusString() == "nvUbdIzC");

    cpu.SetCFlag(false);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("SetVFlag updates only the Overflow flag") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    ResetStatusRegister(cpu);
    CHECK(cpu.StatusString() == "nvUbdIzc");

    cpu.SetVFlag(true);
    CHECK(cpu.StatusString() == "nVUbdIzc");

    cpu.SetVFlag(false);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("Cpu registers start in the documented power-up state") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    CHECK(cpu.GetXRegister() == 0x00);
    CHECK(cpu.GetStatusRegister() == 0x24);
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("LdaImmediate updates the Zero and Negative flags based on the loaded value") {
    nes_test::TestBus bus;
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
    nes_test::TestBus bus;
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

TEST_CASE("Reset loads the Program Counter from the reset vector") {
    const auto rom = nes_test::MakeMinimalRom(0x8123);
    const nes_test::TempRomFile rom_file(rom);
    nes::Cartridge cartridge(rom_file.path());
    nes::Bus bus(cartridge);
    nes::Cpu cpu(bus);

    cpu.SetProgramCounter(0x0000);
    cpu.Reset();

    CHECK(cpu.GetProgramCounter() == 0x8123);
}

TEST_CASE("Step throws for an unimplemented opcode") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xFF); // not a recognized opcode

    CHECK_THROWS_AS((void)cpu.Step(), std::runtime_error);
}

TEST_CASE("Nmi pushes the Program Counter and a masked status register onto the stack") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.SetProgramCounter(0xABCD);
    cpu.SetFlag(nes::Cpu::StatusFlag::B, true);
    cpu.SetFlag(nes::Cpu::StatusFlag::C, true);

    cpu.Nmi();

    // LIFO: the byte pushed last (status) comes off first, then the word (PC).
    const std::uint8_t pushed_status = cpu.StackPullByte();
    CHECK(cpu.StackPullWord() == 0xABCD);

    // The pushed copy always has U set and B cleared, regardless of the live register...
    CHECK((pushed_status & static_cast<std::uint8_t>(nes::Cpu::StatusFlag::U)) != 0);
    CHECK((pushed_status & static_cast<std::uint8_t>(nes::Cpu::StatusFlag::B)) == 0);
    CHECK((pushed_status & static_cast<std::uint8_t>(nes::Cpu::StatusFlag::C)) != 0); // other flags preserved

    // ...but Nmi only pushes a masked copy - it doesn't clear B on the live status register itself.
    CHECK(cpu.IsFlagSet(static_cast<std::uint8_t>(nes::Cpu::StatusFlag::B)));
}

TEST_CASE("Nmi sets the Interrupt Disable flag") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.SetFlag(nes::Cpu::StatusFlag::I, false);
    CHECK_FALSE(cpu.IsFlagSet(static_cast<std::uint8_t>(nes::Cpu::StatusFlag::I)));

    cpu.Nmi();

    CHECK(cpu.IsFlagSet(static_cast<std::uint8_t>(nes::Cpu::StatusFlag::I)));
}

TEST_CASE("Nmi loads the Program Counter from the NMI vector") {
    auto rom = nes_test::MakeMinimalRom();
    const auto prg_start = nes::Cartridge::HEADER_SIZE;
    // NMI vector lives in the last 6 bytes of the 16 KB PRG bank (CPU 0xFFFA/0xFFFB,
    // mirrored down to PRG offset 0x3FFA/0x3FFB by Mapper000's 16 KB mask).
    rom[prg_start + 0x3FFA] = 0x34;
    rom[prg_start + 0x3FFB] = 0x92;

    const nes_test::TempRomFile rom_file(rom);
    nes::Cartridge cartridge(rom_file.path());
    nes::Bus bus(cartridge);
    nes::Cpu cpu(bus);

    cpu.Nmi();

    CHECK(cpu.GetProgramCounter() == 0x9234);
}


