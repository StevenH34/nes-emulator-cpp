#include "Ppu.h"
#include "Cartridge.h"
#include "Ppu_Addresses.h"

namespace nes {

Ppu::Ppu(Cartridge& cartridge) : cartridge_(cartridge) {}

/// Latch methods
/// Returns 1 or 32 depending on bit 2 of PPUCTRL.
/// The & VRAM_MASK makes sure v doesn’t exceed 14 bits.
void Ppu::IncrementVRegister() {
    v_register_ = v_register_ + VramIncrement() & PpuAddresses::VRAM_MASK;
}

/// Puts coarse X in bits 4-0
void Ppu::SetCoarseX(const std::uint16_t coarse_x) {
    t_register_ = t_register_ & CLEAR_COARSE_X | coarse_x;
}

/// Puts the nametable selection in bits 11-10
void Ppu::SetNametable(const std::uint16_t nametable) {
    t_register_ = t_register_ & CLEAR_NAMETABLE | nametable << 10;
}

/// Puts fine Y in bits 14-12 and coarse Y in bits 9-5.
void Ppu::SetScrollY(const std::uint16_t fine_y, const std::uint16_t coarse_y) {
    t_register_ = t_register_ & CLEAR_ALL_Y | fine_y << 12 | coarse_y << 5;
}

/// Ctrl methods
void Ppu::WriteCtrlRegister(const std::uint8_t value) {
    ctrl_register_ = value;
    const auto nametable = static_cast<std::uint16_t>(value & 0x03);
    SetNametable(nametable);
}

std::uint16_t Ppu::VramIncrement() const {
    return (ctrl_register_ & FLAG_VRAM_INCREMENT) != 0 ? 32 : 1;
}

bool Ppu::isNmiEnabled() const {
    return (ctrl_register_ & FLAG_NMI_ENABLED) != 0;
}
} // namespace nes