#include "doctest.h"

#include "../../src/core/Cartridge.h"
#include "../../src/core/mappers/Mapper000.h"
#include "../../src/core/save_state/StateReader.h"
#include "../../src/core/save_state/StateWriter.h"

#include <vector>

namespace {

std::vector<uint8_t> MakePrgRom(const size_t size, const uint8_t first_byte, const uint8_t last_byte) {
  std::vector<uint8_t> prg(size, 0);
  prg.front() = first_byte;
  prg.back() = last_byte;
  return prg;
}

} // namespace

// --- 16KB PRG-ROM (mirrored) ---

TEST_CASE("Mapper000 mirrors a 16KB PRG-ROM across the full $8000-$FFFF range") {
  const auto prg = MakePrgRom(nes::Cartridge::PRG_BLOCK_SIZE, 0x11, 0x22);
  const nes::Mapper000 mapper(prg, {});

  CHECK(mapper.ReadPrg(0x8000) == mapper.ReadPrg(0xC000));
  CHECK(mapper.ReadPrg(0xBFFF) == mapper.ReadPrg(0xFFFF));
}

TEST_CASE("Mapper000 reads the first and last bytes of a 16KB PRG-ROM") {
  const auto prg = MakePrgRom(nes::Cartridge::PRG_BLOCK_SIZE, 0x11, 0x22);
  const nes::Mapper000 mapper(prg, {});

  CHECK(mapper.ReadPrg(0x8000) == 0x11);
  CHECK(mapper.ReadPrg(0xBFFF) == 0x22);
}

// --- 32KB PRG-ROM (linear) ---

TEST_CASE("Mapper000 does not mirror a 32KB PRG-ROM") {
  const auto prg = MakePrgRom(nes::Cartridge::PRG_BLOCK_SIZE * 2, 0x11, 0x22);
  const nes::Mapper000 mapper(prg, {});

  CHECK(mapper.ReadPrg(0x8000) != mapper.ReadPrg(0xC000));
}

TEST_CASE("Mapper000 reads the first and last bytes of a 32KB PRG-ROM") {
  const auto prg = MakePrgRom(nes::Cartridge::PRG_BLOCK_SIZE * 2, 0x11, 0x22);
  const nes::Mapper000 mapper(prg, {});

  CHECK(mapper.ReadPrg(0x8000) == 0x11);
  CHECK(mapper.ReadPrg(0xFFFF) == 0x22);
}

// --- CHR-ROM ---

TEST_CASE("Mapper000 reads the first and last bytes of an 8KB CHR-ROM") {
  const std::vector<uint8_t> prg(nes::Cartridge::PRG_BLOCK_SIZE, 0);
  std::vector<uint8_t> chr(nes::Cartridge::CHR_BLOCK_SIZE, 0);
  chr.front() = 0x33;
  chr.back() = 0x44;
  const nes::Mapper000 mapper(prg, chr);

  CHECK(mapper.ReadChr(0x0000) == 0x33);
  CHECK(mapper.ReadChr(0x1FFF) == 0x44);
}

// --- Save state ---
// Mapper000 has no bank-select registers or other mutable state, so it does
// not override Serialize/Deserialize and inherits Mapper's no-op defaults.

TEST_CASE("Mapper000 Serialize writes no bytes, since NROM has no bank registers to save") {
  const std::vector<uint8_t> prg(nes::Cartridge::PRG_BLOCK_SIZE, 0);
  const nes::Mapper000 mapper(prg, {});

  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);
  mapper.Serialize(writer);

  CHECK(buffer.empty());
}

TEST_CASE("Mapper000 Deserialize does not throw or consume bytes from an unrelated buffer") {
  const std::vector<uint8_t> prg(nes::Cartridge::PRG_BLOCK_SIZE, 0);
  nes::Mapper000 mapper(prg, {});
  const std::vector<uint8_t> buffer = {0x11, 0x22, 0x33};
  nes::StateReader reader(buffer);

  CHECK_NOTHROW(mapper.Deserialize(reader));
  CHECK(reader.BytesRemaining() == buffer.size()); // nothing consumed
}
