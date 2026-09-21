#include "doctest.h"

#include <array>
#include <fstream>
#include <stdexcept>
#include <vector>

#include "../src/core/Cartridge.h"
#include "../src/core/Emulator.h"
#include "../src/core/save_state/StateReader.h"
#include "../src/core/save_state/StateWriter.h"
#include "TestRom.h"

namespace {

// Overwrites an existing file on disk with arbitrary bytes, used to corrupt a
// previously-saved state file for the failure-path tests below.
void OverwriteFile(const std::string& path, const std::vector<uint8_t>& data) {
  std::ofstream file(path, std::ios::binary);
  file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
}

} // namespace

TEST_CASE("Emulator::LoadProgram copies bytes into RAM") {
  const nes_test::TempRomFile rom(nes_test::MakeMinimalRom(0x0200));
  nes::Emulator emulator(rom.path());
  const std::vector<uint8_t> program{0xA9, 0x42, 0xE8};
  constexpr auto start_address = 0x0200;
  emulator.LoadProgram(program, start_address);

  CHECK(emulator.GetBus().ReadRam(0x0200) == 0xA9);
  CHECK(emulator.GetBus().ReadRam(0x0201) == 0x42);
  CHECK(emulator.GetBus().ReadRam(0x0202) == 0xE8);
}

TEST_CASE("Emulator constructor resets the CPU using the cartridge's reset vector") {
  const nes_test::TempRomFile rom(nes_test::MakeMinimalRom(0x0200));
  nes::Emulator emulator(rom.path());

  CHECK(emulator.GetCpu().GetProgramCounter() == 0x0200);
}

TEST_CASE("Emulator::Step executes the loaded program starting at the reset vector") {
  const nes_test::TempRomFile rom(nes_test::MakeMinimalRom(0x0200));
  nes::Emulator emulator(rom.path());
  const std::vector<uint8_t> program{0xA9, 0x42, 0xE8}; // LDA #0x42, INX
  emulator.LoadProgram(program, 0x0200);

  emulator.Step(); // LDA #0x42
  CHECK(emulator.GetCpu().GetAccumulator() == 0x42);

  emulator.Step(); // INX
  CHECK(emulator.GetCpu().GetXRegister() == 0x01);
}

// --- Serialize / Deserialize ---

TEST_CASE("Emulator::Serialize concatenates CPU, PPU, APU, Bus, and Cartridge state, in that order") {
  const nes_test::TempRomFile rom(nes_test::MakeMinimalRom(0x0200));
  nes::Emulator emulator(rom.path());
  emulator.LoadProgram({0xA9, 0x42, 0xE8}, 0x0200); // LDA #0x42, INX
  emulator.Step();
  emulator.Step();

  std::vector<uint8_t> actual;
  nes::StateWriter actual_writer(actual);
  emulator.Serialize(actual_writer);

  std::vector<uint8_t> expected;
  nes::StateWriter expected_writer(expected);
  emulator.GetCpu().Serialize(expected_writer);
  emulator.GetPpu().Serialize(expected_writer);
  emulator.GetApu().Serialize(expected_writer);
  emulator.GetBus().Serialize(expected_writer);
  emulator.GetCartridge().Serialize(expected_writer);

  CHECK(actual == expected);
}

TEST_CASE("Emulator::Deserialize restores CPU registers and RAM to their state at the time of Serialize") {
  const nes_test::TempRomFile rom(nes_test::MakeMinimalRom(0x0200));
  nes::Emulator emulator(rom.path());
  emulator.LoadProgram({0xA9, 0x42, 0xE8, 0xE8, 0xE8}, 0x0200); // LDA #0x42, INX, INX, INX
  emulator.GetBus().WriteRam(0x0300, 0x99);
  emulator.Step(); // LDA
  emulator.Step(); // INX -> X = 1

  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);
  emulator.Serialize(writer);

  const uint8_t saved_accumulator = emulator.GetCpu().GetAccumulator();
  const uint8_t saved_x = emulator.GetCpu().GetXRegister();
  const uint16_t saved_pc = emulator.GetCpu().GetProgramCounter();

  // Diverge from the saved snapshot so Deserialize must actually overwrite
  // state rather than happening to already match it.
  emulator.Step(); // INX -> X = 2
  emulator.Step(); // INX -> X = 3
  emulator.GetBus().WriteRam(0x0300, 0x11);
  REQUIRE(emulator.GetCpu().GetXRegister() != saved_x);

  nes::StateReader reader(buffer);
  emulator.Deserialize(reader);

  CHECK(emulator.GetCpu().GetAccumulator() == saved_accumulator);
  CHECK(emulator.GetCpu().GetXRegister() == saved_x);
  CHECK(emulator.GetCpu().GetProgramCounter() == saved_pc);
  CHECK(emulator.GetBus().ReadRam(0x0300) == 0x99);
}

TEST_CASE("Emulator::Deserialize rolls back to the pre-call state when the payload is truncated partway through") {
  const nes_test::TempRomFile rom(nes_test::MakeMinimalRom(0x0200));
  nes::Emulator emulator(rom.path());
  emulator.LoadProgram({0xA9, 0x42, 0xE8}, 0x0200); // LDA #0x42, INX
  emulator.Step();
  emulator.Step();

  const uint8_t original_accumulator = emulator.GetCpu().GetAccumulator();
  const uint8_t original_x = emulator.GetCpu().GetXRegister();
  const uint16_t original_pc = emulator.GetCpu().GetProgramCounter();

  // Build a truncated payload holding only a *different* CPU snapshot (from a
  // second, independent Emulator), so cpu_.Deserialize succeeds and
  // transiently overwrites emulator's registers before ppu_.Deserialize hits
  // the end of the buffer and throws. If rollback didn't work, the CPU would
  // be left with `other`'s register values instead of its own.
  const nes_test::TempRomFile other_rom(nes_test::MakeMinimalRom(0x0300));
  nes::Emulator other(other_rom.path());
  other.LoadProgram({0xA9, 0x77, 0xE8, 0xE8, 0xE8, 0xE8}, 0x0300); // LDA #0x77, INX x4
  for (int i = 0; i < 5; ++i) {
    other.Step();
  }
  REQUIRE(other.GetCpu().GetAccumulator() != original_accumulator);

  std::vector<uint8_t> truncated_buffer;
  nes::StateWriter writer(truncated_buffer);
  other.GetCpu().Serialize(writer); // CPU bytes only -- PPU/APU/Bus/mapper bytes are missing

  nes::StateReader reader(truncated_buffer);
  CHECK_THROWS_AS(emulator.Deserialize(reader), std::out_of_range);

  CHECK(emulator.GetCpu().GetAccumulator() == original_accumulator);
  CHECK(emulator.GetCpu().GetXRegister() == original_x);
  CHECK(emulator.GetCpu().GetProgramCounter() == original_pc);
}

TEST_CASE("Emulator::Deserialize clears any undrained APU samples left over from before the load") {
  const nes_test::TempRomFile rom(nes_test::MakeMinimalRom());
  nes::Emulator emulator(rom.path());

  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);
  emulator.Serialize(writer);

  emulator.GetApu().GetSampleBuffer().push_back(1.0f);
  emulator.GetApu().GetSampleBuffer().push_back(2.0f);
  REQUIRE(emulator.GetApu().GetSampleBuffer().size() == 2);

  nes::StateReader reader(buffer);
  emulator.Deserialize(reader);

  CHECK(emulator.GetApu().GetSampleBuffer().empty());
}

// --- SaveStateToFile / LoadStateFromFile ---

TEST_CASE("SaveStateToFile writes a header with the NESS magic number, version 1, and the cartridge's mapper "
          "id, ROM sizes, and checksum") {
  const nes_test::TempRomFile rom(nes_test::MakeMinimalRom());
  nes::Emulator emulator(rom.path());
  const nes_test::TempRomFile state_file(std::vector<uint8_t>{});

  emulator.SaveStateToFile(state_file.path());
  const auto raw = nes::Cartridge::ReadFileBytes(state_file.path());
  nes::StateReader reader(raw);

  std::array<uint8_t, 4> magic{};
  reader.ReadBytes(magic);
  CHECK(magic == std::array<uint8_t, 4>{'N', 'E', 'S', 'S'});
  CHECK(reader.ReadU16() == 1); // version
  CHECK(reader.ReadU8() == emulator.GetCartridge().GetMapperId());
  CHECK(reader.ReadU32() == emulator.GetCartridge().GetPrgRom().size());
  CHECK(reader.ReadU32() == emulator.GetCartridge().GetChrRom().size());
  CHECK(reader.ReadU32() == emulator.GetCartridge().GetRomChecksum());
}

TEST_CASE("SaveStateToFile then LoadStateFromFile restores CPU state into a fresh Emulator for the same ROM") {
  const auto rom_data = nes_test::MakeMinimalRom(0x0200);
  const nes_test::TempRomFile rom_a(rom_data);
  const nes_test::TempRomFile rom_b(rom_data); // byte-identical ROM, separate Cartridge instance
  const nes_test::TempRomFile state_file(std::vector<uint8_t>{});

  nes::Emulator source(rom_a.path());
  source.LoadProgram({0xA9, 0x42, 0xE8, 0xE8}, 0x0200); // LDA #0x42, INX, INX
  source.Step();
  source.Step();
  source.Step();
  source.SaveStateToFile(state_file.path());

  nes::Emulator target(rom_b.path());
  target.LoadStateFromFile(state_file.path());

  CHECK(target.GetCpu().GetAccumulator() == source.GetCpu().GetAccumulator());
  CHECK(target.GetCpu().GetXRegister() == source.GetCpu().GetXRegister());
  CHECK(target.GetCpu().GetProgramCounter() == source.GetCpu().GetProgramCounter());
}

TEST_CASE("LoadStateFromFile throws when the ROM checksum does not match the currently loaded cartridge") {
  const nes_test::TempRomFile rom_a(nes_test::MakeMinimalRom(0x0200));
  const nes_test::TempRomFile rom_b(nes_test::MakeMinimalRom(0x0300)); // different reset vector -> different checksum
  const nes_test::TempRomFile state_file(std::vector<uint8_t>{});

  nes::Emulator source(rom_a.path());
  source.SaveStateToFile(state_file.path());

  nes::Emulator target(rom_b.path());
  CHECK_THROWS_AS(target.LoadStateFromFile(state_file.path()), std::runtime_error);
}

TEST_CASE("LoadStateFromFile throws when the file's magic number is wrong") {
  const nes_test::TempRomFile rom(nes_test::MakeMinimalRom());
  nes::Emulator emulator(rom.path());
  const nes_test::TempRomFile state_file(std::vector<uint8_t>{});
  emulator.SaveStateToFile(state_file.path());

  auto raw = nes::Cartridge::ReadFileBytes(state_file.path());
  raw[0] = 0x00; // corrupt the first magic-number byte
  OverwriteFile(state_file.path(), raw);

  CHECK_THROWS_AS(emulator.LoadStateFromFile(state_file.path()), std::runtime_error);
}

TEST_CASE("LoadStateFromFile throws when the file's version does not match") {
  const nes_test::TempRomFile rom(nes_test::MakeMinimalRom());
  nes::Emulator emulator(rom.path());
  const nes_test::TempRomFile state_file(std::vector<uint8_t>{});
  emulator.SaveStateToFile(state_file.path());

  auto raw = nes::Cartridge::ReadFileBytes(state_file.path());
  raw[4] = 0x02; // version low byte: 1 -> 2
  OverwriteFile(state_file.path(), raw);

  CHECK_THROWS_AS(emulator.LoadStateFromFile(state_file.path()), std::runtime_error);
}

TEST_CASE("LoadStateFromFile throws std::runtime_error, not std::out_of_range, when the payload is truncated") {
  const nes_test::TempRomFile rom(nes_test::MakeMinimalRom());
  nes::Emulator emulator(rom.path());
  const nes_test::TempRomFile state_file(std::vector<uint8_t>{});
  emulator.SaveStateToFile(state_file.path());

  auto raw = nes::Cartridge::ReadFileBytes(state_file.path());
  raw.resize(raw.size() - 1); // header stays intact, but the payload is short by one byte
  OverwriteFile(state_file.path(), raw);

  // A truncated payload makes StateReader throw std::out_of_range internally;
  // LoadStateFromFile must convert that into std::runtime_error like every
  // other corrupt-file case, not let it leak out as a different exception type.
  CHECK_THROWS_AS(emulator.LoadStateFromFile(state_file.path()), std::runtime_error);
}

TEST_CASE("LoadStateFromFile throws when the save state file does not exist") {
  const nes_test::TempRomFile rom(nes_test::MakeMinimalRom());
  nes::Emulator emulator(rom.path());

  CHECK_THROWS_AS(emulator.LoadStateFromFile("/no/such/file/definitely_missing.state"), std::runtime_error);
}
