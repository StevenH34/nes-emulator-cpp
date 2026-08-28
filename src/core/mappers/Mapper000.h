#pragma once

#include "Mapper.h"

#include <vector>

namespace nes {

/// Mapper 000, NROM (no bank switch)
class Mapper000 final : public Mapper {
public:
  Mapper000(std::span<const uint8_t> prg_rom, std::span<const uint8_t> chr_rom);
  /// PRG-ROM: 16KB mirrored or 32KB linear
  [[nodiscard]] uint8_t ReadPrg(uint16_t address) const override;
  /// CHR-ROM: 8KB linear
  [[nodiscard]] uint8_t ReadChr(uint16_t address) const override;

private:
  static constexpr uint16_t PRG_MASK_16K = 0X3FFF;
  static constexpr uint16_t PRG_MASK_32K = 0X7FFF;
  static constexpr uint16_t CHR_MASK = 0x1FFF;

  std::vector<uint8_t> prg_rom_;
  std::vector<uint8_t> chr_rom_;
  uint16_t prg_mask_;
};

} // namespace nes
