#include "doctest.h"

#include "../src/core/Cartridge.h"
#include "../src/core/save_state/StateReader.h"
#include "../src/core/save_state/StateWriter.h"
#include "TestRom.h"

#include <stdexcept>
#include <vector>

namespace {

std::vector<uint8_t> MakeHeader(const uint8_t prg_blocks, const uint8_t chr_blocks, const uint8_t flags_6,
                                const uint8_t flags_7) {
  return {0x4E, 0x45, 0x53, 0x1A, prg_blocks, chr_blocks, flags_6, flags_7, 0, 0, 0, 0, 0, 0, 0, 0};
}

// A minimal, single-bank mapper 0 (NROM) ROM, valid enough to construct a Cartridge.
std::vector<uint8_t> MakeMinimalRomData() {
  auto data = MakeHeader(1, 1, 0, 0);
  data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE + nes::Cartridge::CHR_BLOCK_SIZE, 0);
  return data;
}

} // namespace

using nes_test::TempRomFile;

// --- ParseMapperId ---

TEST_CASE("ParseMapperId returns 0 when both nibbles are 0") { CHECK(nes::Cartridge::ParseMapperId(0x00, 0x00) == 0); }

TEST_CASE("ParseMapperId reads the low nibble from the top bits of flags_6") {
  CHECK(nes::Cartridge::ParseMapperId(0x10, 0x00) == 0x01);
}

TEST_CASE("ParseMapperId reads the high nibble from the top bits of flags_7") {
  CHECK(nes::Cartridge::ParseMapperId(0x00, 0x10) == 0x10);
}

TEST_CASE("ParseMapperId combines both nibbles into the full mapper number") {
  // flags_6 top nibble = 0xA, flags_7 top nibble = 0x5 -> mapper 0x5A (90)
  CHECK(nes::Cartridge::ParseMapperId(0xA0, 0x50) == 0x5A);
}

TEST_CASE("ParseMapperId ignores the lower flag bits of flags_6 and flags_7") {
  // Lower nibble of flags_6 carries mirroring/battery/trainer bits and must not
  // leak into the mapper id
  CHECK(nes::Cartridge::ParseMapperId(0xA3, 0x5C) == 0x5A);
}

// --- ParseMirroring ---

TEST_CASE("ParseMirroring returns Horizontal when no mirroring bits are set") {
  CHECK(nes::Cartridge::ParseMirroring(0x00) == nes::Cartridge::Mirroring::Horizontal);
}

TEST_CASE("ParseMirroring returns Vertical when the mirror bit is set") {
  CHECK(nes::Cartridge::ParseMirroring(nes::Cartridge::MIRROR_MASK) == nes::Cartridge::Mirroring::Vertical);
}

TEST_CASE("ParseMirroring returns FourScreen when the four-screen bit is set") {
  CHECK(nes::Cartridge::ParseMirroring(nes::Cartridge::FOUR_SCREEN_MASK) == nes::Cartridge::Mirroring::FourScreen);
}

TEST_CASE("ParseMirroring gives FourScreen priority over the mirror bit") {
  const uint8_t flags_6 = nes::Cartridge::FOUR_SCREEN_MASK | nes::Cartridge::MIRROR_MASK;
  CHECK(nes::Cartridge::ParseMirroring(flags_6) == nes::Cartridge::Mirroring::FourScreen);
}

// --- ValidateHeader ---

TEST_CASE("ValidateHeader accepts a well-formed header") {
  const auto data = MakeHeader(1, 1, 0, 0);
  CHECK_NOTHROW(nes::Cartridge::ValidateHeader(data));
}

TEST_CASE("ValidateHeader throws when the data is smaller than the header") {
  const std::vector<uint8_t> data(10, 0);
  CHECK_THROWS_AS(nes::Cartridge::ValidateHeader(data), std::runtime_error);
}

TEST_CASE("ValidateHeader throws when the magic number is wrong") {
  auto data = MakeHeader(1, 1, 0, 0);
  data[0] = 0x00;
  CHECK_THROWS_AS(nes::Cartridge::ValidateHeader(data), std::runtime_error);
}

// --- ReadFileBytes ---

TEST_CASE("ReadFileBytes returns the exact bytes on disk") {
  const std::vector<uint8_t> expected = {0x4E, 0x45, 0x53, 0x1A, 0xDE, 0xAD, 0xBE, 0xEF};
  const TempRomFile rom(expected);

  const auto actual = nes::Cartridge::ReadFileBytes(rom.path());

  CHECK(actual == expected);
}

TEST_CASE("ReadFileBytes throws when the file does not exist") {
  CHECK_THROWS_AS(nes::Cartridge::ReadFileBytes("/no/such/file/definitely_missing.nes"), std::runtime_error);
}

// --- Cartridge construction ---

TEST_CASE("Cartridge parses a minimal one-bank ROM with no trainer") {
  auto data = MakeHeader(1, 1, 0, 0);
  data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE + nes::Cartridge::CHR_BLOCK_SIZE, 0);
  data[16] = 0x11; // first PRG-ROM byte
  data[16 + nes::Cartridge::PRG_BLOCK_SIZE] = 0x22; // first CHR-ROM byte
  const TempRomFile rom(data);

  const nes::Cartridge cart(rom.path());

  CHECK(cart.GetPath() == rom.path());
  CHECK(cart.GetPrgRom().size() == nes::Cartridge::PRG_BLOCK_SIZE);
  CHECK(cart.GetChrRom().size() == nes::Cartridge::CHR_BLOCK_SIZE);
  CHECK(cart.GetPrgRom()[0] == 0x11);
  CHECK(cart.GetChrRom()[0] == 0x22);
  CHECK(cart.GetMirroring() == nes::Cartridge::Mirroring::Horizontal);
  CHECK_FALSE(cart.HasBatteryBackedRam());
}

TEST_CASE("Cartridge synthesizes 8KB of zeroed CHR-RAM when CHR-ROM size is 0") {
  auto data = MakeHeader(1, 0, 0, 0);
  data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE, 0);
  const TempRomFile rom(data);

  const nes::Cartridge cart(rom.path());

  CHECK(cart.GetChrRom().size() == nes::Cartridge::CHR_BLOCK_SIZE);
  CHECK(cart.GetChrRom() == std::vector<uint8_t>(nes::Cartridge::CHR_BLOCK_SIZE, 0));
}

TEST_CASE("Cartridge skips the 512-byte trainer before reading PRG-ROM") {
  auto data = MakeHeader(1, 1, nes::Cartridge::TRAINER_MASK, 0);
  data.resize(data.size() + nes::Cartridge::TRAINER_SIZE,
              0xFF); // trainer bytes
  data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE + nes::Cartridge::CHR_BLOCK_SIZE, 0);
  const size_t prg_start = nes::Cartridge::HEADER_SIZE + nes::Cartridge::TRAINER_SIZE;
  data[prg_start] = 0x77;
  const TempRomFile rom(data);

  const nes::Cartridge cart(rom.path());

  CHECK(cart.GetPrgRom().size() == nes::Cartridge::PRG_BLOCK_SIZE);
  CHECK(cart.GetPrgRom()[0] == 0x77);
}

TEST_CASE("Cartridge reports battery-backed RAM when the battery flag is set") {
  auto data = MakeHeader(1, 1, nes::Cartridge::BATTERY_MASK, 0);
  data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE + nes::Cartridge::CHR_BLOCK_SIZE, 0);
  const TempRomFile rom(data);

  const nes::Cartridge cart(rom.path());

  CHECK(cart.HasBatteryBackedRam());
}

TEST_CASE("Cartridge constructor throws when the mapper id is unsupported") {
  // flags_6 top nibble = 0xA, flags_7 top nibble = 0x5 -> mapper 0x5A (90),
  // which has no Mapper implementation
  auto data = MakeHeader(1, 1, 0xA0, 0x50);
  data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE + nes::Cartridge::CHR_BLOCK_SIZE, 0);
  const TempRomFile rom(data);

  CHECK_THROWS_AS(nes::Cartridge(rom.path()), std::runtime_error);
}

TEST_CASE("Cartridge constructs a mapper wired to the parsed PRG-ROM and "
          "CHR-ROM data") {
  auto data = MakeHeader(1, 1, 0, 0); // mapper id 0 (NROM)
  data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE + nes::Cartridge::CHR_BLOCK_SIZE, 0);
  data[16] = 0x11; // first PRG-ROM byte
  data[16 + nes::Cartridge::PRG_BLOCK_SIZE] = 0x22; // first CHR-ROM byte
  const TempRomFile rom(data);

  const nes::Cartridge cart(rom.path());

  CHECK(cart.GetMapperId() == 0);
  CHECK(cart.GetMapper().ReadPrg(0x8000) == 0x11);
  CHECK(cart.GetMapper().ReadChr(0x0000) == 0x22);
}

TEST_CASE("Cartridge parses vertical mirroring from flags_6") {
  auto data = MakeHeader(1, 1, nes::Cartridge::MIRROR_MASK, 0);
  data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE + nes::Cartridge::CHR_BLOCK_SIZE, 0);
  const TempRomFile rom(data);

  const nes::Cartridge cart(rom.path());

  CHECK(cart.GetMirroring() == nes::Cartridge::Mirroring::Vertical);
}

TEST_CASE("Cartridge constructor throws when the file is too small to be an "
          "iNES file") {
  const TempRomFile rom(std::vector<uint8_t>(10, 0));
  CHECK_THROWS_AS(nes::Cartridge(rom.path()), std::runtime_error);
}

TEST_CASE("Cartridge constructor throws when the magic number is missing") {
  auto data = MakeHeader(1, 1, 0, 0);
  data[0] = 0x00;
  data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE + nes::Cartridge::CHR_BLOCK_SIZE, 0);
  const TempRomFile rom(data);

  CHECK_THROWS_AS(nes::Cartridge(rom.path()), std::runtime_error);
}

TEST_CASE("Cartridge constructor throws when the file is truncated before the "
          "declared PRG/CHR data ends") {
  // Header claims 1 PRG bank and 1 CHR bank but the file only contains the
  // header
  const TempRomFile rom(MakeHeader(1, 1, 0, 0));
  CHECK_THROWS_AS(nes::Cartridge(rom.path()), std::runtime_error);
}

TEST_CASE("Cartridge constructor throws when the file does not exist") {
  CHECK_THROWS_AS(nes::Cartridge("/no/such/file/definitely_missing.nes"), std::runtime_error);
}

// --- Save state ---
// Cartridge itself has no mutable state to save (the ROM data is re-read from
// disk on load); Serialize/Deserialize just delegate to the mapper's own
// save/load, since only the mapper carries runtime state (bank registers, etc).

TEST_CASE("Cartridge Serialize delegates to the mapper, producing the same bytes as "
          "calling Serialize on the mapper directly") {
  const TempRomFile rom(MakeMinimalRomData());
  const nes::Cartridge cart(rom.path());

  std::vector<uint8_t> via_cartridge;
  nes::StateWriter cartridge_writer(via_cartridge);
  cart.Serialize(cartridge_writer);

  std::vector<uint8_t> via_mapper;
  nes::StateWriter mapper_writer(via_mapper);
  cart.GetMapper().Serialize(mapper_writer);

  CHECK(via_cartridge == via_mapper);
}

TEST_CASE("Cartridge Serialize writes no bytes, since Mapper000 has no serializable state") {
  const TempRomFile rom(MakeMinimalRomData());
  const nes::Cartridge cart(rom.path());

  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);
  cart.Serialize(writer);

  CHECK(buffer.empty());
}

TEST_CASE("Cartridge Deserialize does not throw when given an empty saved buffer") {
  const TempRomFile rom(MakeMinimalRomData());
  nes::Cartridge cart(rom.path());
  const std::vector<uint8_t> buffer;
  nes::StateReader reader(buffer);

  CHECK_NOTHROW(cart.Deserialize(reader));
}

TEST_CASE("Cartridge save/load round-trip leaves the mapper readable and functional") {
  const auto data = MakeMinimalRomData();
  const TempRomFile rom(data);
  const nes::Cartridge cart(rom.path());

  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);
  cart.Serialize(writer);

  nes::Cartridge restored(rom.path());
  nes::StateReader reader(buffer);
  restored.Deserialize(reader);

  CHECK(restored.GetMapper().ReadPrg(0x8000) == cart.GetMapper().ReadPrg(0x8000));
  CHECK(restored.GetMapper().ReadChr(0x0000) == cart.GetMapper().ReadChr(0x0000));
}

// --- ComputeRomChecksum / GetRomChecksum ---
// FNV-1a 32-bit over PRG-ROM bytes followed by CHR-ROM bytes; expected values
// below were computed independently against the same algorithm.

TEST_CASE("ComputeRomChecksum returns the FNV-1a offset basis when both spans are empty") {
  CHECK(nes::Cartridge::ComputeRomChecksum({}, {}) == 0x811C9DC5u);
}

TEST_CASE("ComputeRomChecksum folds a single PRG-ROM byte into the offset basis") {
  const std::vector<uint8_t> prg{0x11};
  CHECK(nes::Cartridge::ComputeRomChecksum(prg, {}) == 0x140C74BCu);
}

TEST_CASE("ComputeRomChecksum folds a single CHR-ROM byte into the offset basis") {
  const std::vector<uint8_t> chr{0x22};
  CHECK(nes::Cartridge::ComputeRomChecksum({}, chr) == 0x270C92A5u);
}

TEST_CASE("ComputeRomChecksum folds PRG-ROM bytes before CHR-ROM bytes") {
  const std::vector<uint8_t> prg{0x11, 0x22, 0x33};
  const std::vector<uint8_t> chr{0x44, 0x55};
  CHECK(nes::Cartridge::ComputeRomChecksum(prg, chr) == 0x0A2F16B8u);
}

TEST_CASE("ComputeRomChecksum is sensitive to byte order, not just byte content") {
  const std::vector<uint8_t> a{0x01, 0x02};
  const std::vector<uint8_t> b{0x02, 0x01};
  CHECK(nes::Cartridge::ComputeRomChecksum(a, {}) != nes::Cartridge::ComputeRomChecksum(b, {}));
}

TEST_CASE("Cartridge stores GetRomChecksum computed from its own parsed PRG-ROM and CHR-ROM") {
  auto data = MakeHeader(1, 1, 0, 0);
  data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE + nes::Cartridge::CHR_BLOCK_SIZE, 0);
  data[16] = 0x11; // first PRG-ROM byte
  data[16 + nes::Cartridge::PRG_BLOCK_SIZE] = 0x22; // first CHR-ROM byte
  const TempRomFile rom(data);

  const nes::Cartridge cart(rom.path());

  CHECK(cart.GetRomChecksum() == nes::Cartridge::ComputeRomChecksum(cart.GetPrgRom(), cart.GetChrRom()));
}

TEST_CASE("Cartridge GetRomChecksum matches for two Cartridges parsed from byte-identical ROM files") {
  const auto data = MakeMinimalRomData();
  const TempRomFile rom_a(data);
  const TempRomFile rom_b(data);

  const nes::Cartridge cart_a(rom_a.path());
  const nes::Cartridge cart_b(rom_b.path());

  CHECK(cart_a.GetRomChecksum() == cart_b.GetRomChecksum());
}

TEST_CASE("Cartridge GetRomChecksum differs when PRG-ROM content differs") {
  auto data_a = MakeHeader(1, 1, 0, 0);
  data_a.resize(data_a.size() + nes::Cartridge::PRG_BLOCK_SIZE + nes::Cartridge::CHR_BLOCK_SIZE, 0);
  auto data_b = data_a;
  data_b[16] = 0xFF; // change the first PRG-ROM byte only

  const TempRomFile rom_a(data_a);
  const TempRomFile rom_b(data_b);
  const nes::Cartridge cart_a(rom_a.path());
  const nes::Cartridge cart_b(rom_b.path());

  CHECK(cart_a.GetRomChecksum() != cart_b.GetRomChecksum());
}
