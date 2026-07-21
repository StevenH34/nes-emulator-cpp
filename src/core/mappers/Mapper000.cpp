#include "Mapper000.h"

#include "Cartridge.h"

namespace nes {

Mapper000::Mapper000(std::span<const std::uint8_t> prg_rom, std::span<const std::uint8_t> chr_rom)
    : prg_rom_(prg_rom.begin(), prg_rom.end()),
      chr_rom_(chr_rom.begin(), chr_rom.end()),
      prg_mask_(prg_rom_.size() > Cartridge::PRG_BLOCK_SIZE ? PRG_MASK_32K : PRG_MASK_16K) {
}

std::uint8_t Mapper000::ReadPrg(const std::uint16_t address) const {
    return prg_rom_[address & prg_mask_];
}

std::uint8_t Mapper000::ReadChr(const std::uint16_t address) const {
    return chr_rom_[address & CHR_MASK];
}

}// namespace nes