#include "doctest.h"

#include <stdexcept>
#include <vector>

#include "../src/core/Bus.h"
#include "../src/core/Cartridge.h"
#include "../src/core/Controller.h"
#include "../src/core/ppu/Ppu.h"
#include "../src/core/save_state/StateReader.h"
#include "../src/core/save_state/StateWriter.h"
#include "TestBus.h"
#include "TestRom.h"

TEST_CASE("Bus RAM reads and writes are mirrored across the 2 KB window") {
  nes_test::TestBus bus;

  bus.WriteCpu(0x07FF, 0xAB);

  CHECK(bus.ReadCpu(0x07FF) == 0xAB);
  CHECK(bus.ReadCpu(0x0FFF) == 0xAB);
  CHECK(bus.ReadCpu(0x17FF) == 0xAB);
  CHECK(bus.ReadCpu(0x1FFF) == 0xAB);
}

TEST_CASE("Bus RAM read and write helpers access the mirrored backing store") {
  nes_test::TestBus bus;

  bus.WriteRam(0x0000, 0x11);
  bus.WriteRam(0x0800, 0x22);

  CHECK(bus.ReadRam(0x0000) == 0x22);
  CHECK(bus.ReadRam(0x0800) == 0x22);
  CHECK(bus.ReadRam(0x1000) == 0x22);
  CHECK(bus.ReadRam(0x1800) == 0x22);
}

TEST_CASE("Bus RAM starts zero-initialized") {
  nes_test::TestBus bus;

  CHECK(bus.ReadCpu(0x0000) == 0x00);
  CHECK(bus.ReadCpu(0x07FF) == 0x00);
}

TEST_CASE("Bus writes do not bleed into unrelated, non-mirrored addresses") {
  nes_test::TestBus bus;

  bus.WriteCpu(0x0010, 0x99);

  CHECK(bus.ReadCpu(0x0010) == 0x99);
  CHECK(bus.ReadCpu(0x0011) == 0x00);
  CHECK(bus.ReadCpu(0x0800) == 0x00); // mirror of 0x0000, not 0x0010
}

TEST_CASE("Bus returns zero for the unmapped region between RAM and PRG-ROM") {
  nes_test::TestBus bus;

  // 0x2000-0x7FFF is PPU/APU/expansion/SRAM space, none of which Bus wires up
  // yet.
  CHECK(bus.ReadCpu(0x2000) == 0x00);
  CHECK(bus.ReadCpu(0x4000) == 0x00);
  CHECK(bus.ReadCpu(0x7FFF) == 0x00);
}

TEST_CASE("Bus ignores writes to the unmapped region between RAM and PRG-ROM") {
  nes_test::TestBus bus;

  bus.WriteCpu(0x2000, 0xAB);

  CHECK(bus.ReadCpu(0x2000) == 0x00);
  // A naive address-mask without a range check would alias 0x2000 onto 0x0000
  CHECK(bus.ReadCpu(0x0000) == 0x00);
}

TEST_CASE("Bus reads PRG-ROM through the cartridge mapper at 0x8000-0xFFFF") {
  std::vector<uint8_t> data{0x4E, 0x45, 0x53, 0x1A, 0x01, 0x01, 0x00, 0x00, 0, 0, 0, 0, 0, 0, 0, 0};
  data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE + nes::Cartridge::CHR_BLOCK_SIZE, 0);
  // Marker byte at PRG offset 0, mirrored by Mapper000's 16 KB mask onto both
  // CPU 0x8000 and 0xC000, so this proves the read comes from the mapper and
  // not from a coincidentally-zeroed backing array.
  data[nes::Cartridge::HEADER_SIZE] = 0x42;

  const nes_test::TempRomFile rom(data);
  nes::Cartridge cartridge(rom.path());
  nes::Ppu ppu(cartridge);
  nes::Apu apu;
  const nes::Bus bus(cartridge, ppu, apu);

  CHECK(bus.ReadCpu(0x8000) == 0x42);
  CHECK(bus.ReadCpu(0xC000) == 0x42);
}

TEST_CASE("Bus Serialize writes RAM followed by controller 1 and controller 2 state, in order") {
  nes_test::TestBus bus;
  for (uint16_t address = 0; address < 2048; ++address) {
    bus.WriteRam(address, static_cast<uint8_t>(address * 7 + 3));
  }
  bus.GetController1().Press(nes::Controller::BUTTON_A);
  bus.GetController1().Write(1); // strobe held high, latches buttons_ into shift_register_
  bus.GetController2().Press(nes::Controller::BUTTON_START);
  bus.GetController2().Write(1);
  bus.GetController2().Write(0); // latch, then freeze

  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);
  bus.Serialize(writer);

  REQUIRE(buffer.size() == 2048 + 3 + 3);
  for (uint16_t address = 0; address < 2048; ++address) {
    CHECK(buffer[address] == static_cast<uint8_t>(address * 7 + 3));
  }
  // Controller 1: buttons_, shift_register_, strobe_ -- still strobing.
  CHECK(buffer[2048] == nes::Controller::BUTTON_A);
  CHECK(buffer[2049] == nes::Controller::BUTTON_A);
  CHECK(buffer[2050] == 1);
  // Controller 2: buttons_, shift_register_, strobe_ -- latched then frozen.
  CHECK(buffer[2051] == nes::Controller::BUTTON_START);
  CHECK(buffer[2052] == nes::Controller::BUTTON_START);
  CHECK(buffer[2053] == 0);
}

TEST_CASE("Bus save/load round-trip restores RAM and both controllers, overwriting prior state") {
  nes_test::TestBus bus;
  for (uint16_t address = 0; address < 2048; ++address) {
    bus.WriteRam(address, static_cast<uint8_t>(address ^ 0xA5));
  }
  bus.GetController1().Press(nes::Controller::BUTTON_A);
  bus.GetController1().Write(1);
  bus.GetController1().Write(0); // latch, then freeze
  bus.GetController2().Press(nes::Controller::BUTTON_START);
  bus.GetController2().Write(1); // strobe held high, never latched to low

  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);
  bus.Serialize(writer);

  nes_test::TestBus restored;
  // Mutate the restored bus to different state first, so Deserialize must
  // overwrite it rather than happening to already match.
  for (uint16_t address = 0; address < 2048; ++address) {
    restored.WriteRam(address, 0xFF);
  }
  restored.GetController1().Press(nes::Controller::BUTTON_B);
  restored.GetController1().Write(1);
  restored.GetController2().Press(nes::Controller::BUTTON_SELECT);

  nes::StateReader reader(buffer);
  restored.Deserialize(reader);

  for (uint16_t address = 0; address < 2048; ++address) {
    CHECK(restored.ReadRam(address) == static_cast<uint8_t>(address ^ 0xA5));
  }
  CHECK(restored.GetController1().Read() == 1); // A, frozen after latch
  // Controller 2's active strobe survived the round-trip independently of
  // controller 1's frozen one, proving the two weren't swapped.
  CHECK(restored.GetController2().Read() == 0); // live read, only START was pressed
  restored.GetController2().Press(nes::Controller::BUTTON_A);
  CHECK(restored.GetController2().Read() == 1);
}

TEST_CASE("Bus Deserialize throws when the saved buffer is truncated") {
  nes_test::TestBus bus;
  const std::vector<uint8_t> buffer(10, 0); // far fewer than the 2048+3+3 bytes Serialize writes
  nes::StateReader reader(buffer);

  CHECK_THROWS_AS(bus.Deserialize(reader), std::out_of_range);
}
