#ifndef NES_EMULATOR_CPP_MAPPER000_H
#define NES_EMULATOR_CPP_MAPPER000_H

#include "Mapper.h"

namespace nes {

/// Mapper 000, NROM (no bank switch)
class Mapper000 final: public Mapper {
public:
    Mapper000(std::span<const std::uint8_t> prg_rom, std::span<const std::uint8_t> chr_rom);
    /// PRG-ROM: 16KB mirrored or 32KB linear
    [[nodiscard]] std::uint8_t ReadPrg(std::uint16_t address) const override;
    /// CHR-ROM: 8KB linear
    [[nodiscard]] std::uint8_t ReadChr(std::uint16_t address) const override;
private:
    static constexpr std::uint16_t PRG_MASK_16K = 0X3FFF;
    static constexpr std::uint16_t PRG_MASK_32K = 0X7FFF;
    static constexpr std::uint16_t CHR_MASK = 0x1FFF;

    std::vector<std::uint8_t> prg_rom_;
    std::vector<std::uint8_t> chr_rom_;
    std::uint16_t prg_mask_;
};

} // namespace nes

#endif //NES_EMULATOR_CPP_MAPPER000_H
