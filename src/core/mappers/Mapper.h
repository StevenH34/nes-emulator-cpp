#ifndef NES_EMULATOR_CPP_MAPPER_H
#define NES_EMULATOR_CPP_MAPPER_H

#include <cstdint>
#include <memory>
#include <span>

namespace nes {

/// Abstract Mapper Class
class Mapper {
public:
  virtual ~Mapper() = default;
  Mapper() = default;
  Mapper(const Mapper&) = delete;
  Mapper& operator=(const Mapper&) = delete;
  Mapper(Mapper&&) = default;
  Mapper& operator=(Mapper&&) = default;

  /// Construct the Mapper subclass for the given iNES mapper ID
  /// @param id The iNES mapper number
  /// @param prg_rom View of the cartridge's PRG-ROM data
  /// @param chr_rom View of the cartridge's CHR-ROM data
  /// @throws std::runtime_error if the mapper ID is unsupported
  static std::unique_ptr<Mapper> Create(uint8_t id, std::span<const uint8_t> prg_rom, std::span<const uint8_t> chr_rom);

  /// Read a byte from the PRG-ROM ($8000-$FFFF)
  /// @param address The address to read from
  /// @return The byte read from the PRG-ROM
  [[nodiscard]] virtual uint8_t ReadPrg(uint16_t address) const = 0;

  /// CPU writes to mapper ($8000-$FFFF)
  /// @param address The address to write to
  /// @param value The value to write
  virtual void WritePrg(uint16_t /*address*/, uint8_t /*value*/) {
    // Default: ignore writes (e.g. Mapper 0 has no registers)
  }

  /// PPU reads CHR-ROM ($0000-$1FFF)
  /// @param address The address to read from
  /// @return The byte read from the CHR-ROM
  [[nodiscard]] virtual uint8_t ReadChr(uint16_t address) const = 0;

  /// PPU writes to CHR if CHR-RAM is present
  /// @param address The address to write to
  /// @param value The value to write
  virtual void WriteChr(uint16_t /*address*/, uint8_t /*value*/) {
    // Default: ignore writes (most mappers don't have CHR-RAM)
  }

  /// CPU reads from WRAM ($6000-$7FFF)
  /// @param address The address to read from
  /// @return The byte read from the WRAM
  [[nodiscard]] virtual uint8_t ReadWram(uint16_t /*address*/) const { return 0; }

  /// CPU writes to WRAM
  /// @param address The address to write to
  /// @param value The value to write
  virtual void WriteWram(uint16_t /*address*/, uint8_t /*value*/) {
    // Default: ignore writes (no WRAM is present)
  }
};

} // namespace nes

#endif // NES_EMULATOR_CPP_MAPPER_H
