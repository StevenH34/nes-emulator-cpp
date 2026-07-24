#include "Bus.h"

#include <iostream>
#include <ostream>

namespace nes {

Bus::Bus(Cartridge& cartridge, Ppu& ppu) : cartridge_(cartridge), ppu_(ppu) {} ;

std::uint8_t Bus::ReadCpu(const std::uint16_t address) const {
    if (address >= RAM_START && address <= RAM_MIRROR_END) {
        return ReadRam(address);
    }
    if (address >= PPU_START && address <= PPU_MIRROR_END) {
        return ppu_.ReadRegister(address);
    }
    if (address >= PRG_ROM_START && address <= PRG_ROM_END) {
        return cartridge_.GetMapper().ReadPrg(address);
    }
    if (address == CONTROLLER_1) {
        return controller_1_.Read();
    }
    if (address == CONTROLLER_2) {
        return controller_2_.Read();
    }
    if (address >= PRG_ROM_START && address <= PRG_ROM_END) {
        return cartridge_.GetMapper().ReadPrg(address);
    }

    return 0;
}

void Bus::WriteCpu(const std::uint16_t address, const std::uint8_t value) {
    if (address >= RAM_START && address <= RAM_MIRROR_END) {
        WriteRam(address, value);
    }
    if (address >= PPU_START && address <= PPU_MIRROR_END) {
        ppu_.WriteRegister(address, value);
    }
    if (address == OAM_DMA) {
        OamDma(value);
    }
    if (address == CONTROLLER_1) {
        // When the game writes to $4016, the strobe goes to both controllers.
        controller_1_.Write(value);
        controller_2_.Write(value);
    }
}

std::uint8_t Bus::ReadRam(const std::uint16_t address) const {
    const auto mirrored_address = (address & RAM_MASK);
    return ram_[mirrored_address];
}

void Bus::WriteRam(const std::uint16_t address, const std::uint8_t value) {
    const auto mirrored_address = (address & RAM_MASK);
    ram_[mirrored_address] = value;
}

/// Copy 256 bytes from CPU page $XX00 to PPU OAM memory.
void Bus::OamDma(const std::uint8_t page) const {
    const auto base = static_cast<std::uint16_t>(page) << 8;
    std::array<std::uint8_t, 256> data{};
    for (std::size_t i = 0; i < 256; ++i) {
        data[i] = ReadCpu(static_cast<std::uint16_t>(base) + i);
    }
    ppu_.OamDma(data);
}
} // nes