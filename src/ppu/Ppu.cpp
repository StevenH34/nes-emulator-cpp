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
/// PPUCTRL configures the PPU
/// bit 0-1: base nametable (0=$2000, 1=$2400, 2=$2800, 3=$2C00)
/// bit 2: VRAM increment (0=+1 horizontal, 1=+32 vertical)
/// bit 3: sprite pattern table (0=$0000, 1=$1000)
/// bit 4: background pattern table (0=$0000, 1=$1000)
/// bit 5: sprite size (0=8x8, 1=8x16)
/// bit 6: master/slave (not used on NES)
/// bit 7: generate NMI on VBlank
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

/// Status register methods
std::uint8_t Ppu::ReadStatusRegister() {
    const std::uint8_t status_snapshot = status_register_;
    // Clear the VBlank flag
    ClearVblank();
    // Reset the latch
    ResetLatch();
    return status_snapshot;
}

void Ppu::SetVblank() {
    status_register_ |= FLAG_VBLANK;
}

void Ppu::ClearVblank() {
    status_register_ &= ~FLAG_VBLANK;
}

} // namespace nes