#include "Ppu.h"
#include "Cartridge.h"
#include "Ppu_Addresses.h"

namespace nes {

Ppu::Ppu(Cartridge& cartridge) : cartridge_(cartridge) {}

/// Latch methods
/// Returns 1 or 32 depending on bit 2 of PPUCTRL.
/// The & VRAM_MASK makes sure v doesn't exceed 14 bits.
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

// Checks bit 4 of PPUCTRL. If set, background tiles live in pattern table 1 ($1000),
// otherwise in pattern table 0 ($0000).
std::uint16_t Ppu::BackgroundPatternTable() const {
    return (ctrl_register_ & FLAG_BG_PATTERN_TABLE) != 0 ? PATTERN_TABLE_1 : PATTERN_TABLE_0;
}

// Checks bit 3 of PPUCTRL. If set, sprite tiles live in pattern table 1 ($1000),
// otherwise in pattern table 0 ($0000).
std::uint16_t Ppu::SpritePatternTable() const {
    return (ctrl_register_ & FLAG_SPR_PATTERN_TABLE) != 0 ? PATTERN_TABLE_1 : PATTERN_TABLE_0;
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

/// Scroll register methods - PPUSCROLL ($2005)
void Ppu::WriteScroll(const std::uint8_t value) {
    IsLatchOn() ? WriteScrollY(value) : WriteScrollX(value);
    ToggleLatch();
}

void Ppu::WriteScrollX(const std::uint8_t value) {
    x_register_ = value & FINE_BITS;
    const std::uint16_t coarse_x = static_cast<std::uint16_t>(value) >> 3;
    SetCoarseX(coarse_x);
}

void Ppu::WriteScrollY(const std::uint8_t value) {
    const std::uint16_t fine_y = static_cast<std::uint16_t>(value) & FINE_BITS;
    const std::uint16_t coarse_y = static_cast<std::uint16_t>(value) >> 3;
    SetScrollY(fine_y, coarse_y);
}

/// PPUADDR ($2006): VRAM address
void Ppu::WriteAddr(const std::uint8_t value) {
    if (IsLatchOn()) {
        t_register_ = t_register_ & 0xFF00 | static_cast<std::uint16_t>(value);
        v_register_ = t_register_;
    } else {
        t_register_ = t_register_ & 0x00FF | (static_cast<std::uint16_t>(value) & 0x3F) << 8;
    }
    ToggleLatch();
}

/// PPUDATA ($2007): VRAM access
std::uint8_t Ppu::ReadDataRegister() {
    std::uint8_t result;
    if (v_register_ >= PpuAddresses::PALETTE_START) {
        result = ReadVram(v_register_);
        vram_buffer_ = ReadVram(v_register_ - 0x1000);
    } else {
        result = vram_buffer_;
        vram_buffer_ = ReadVram(v_register_);
    }
    IncrementVRegister();
    return result;
}

void Ppu::WriteData(const std::uint8_t value) {
    WriteVram(v_register_ ,value);
    IncrementVRegister();
}

/// OAMDATA ($2004): Sprites
void Ppu::WriteOamData(const std::uint8_t value) {
    oam[oam_addr_register_] = value;
    oam_addr_register_ += 1;
}

/// Register router
std::uint8_t Ppu::ReadRegister(const std::uint16_t address) {
    switch (address & 0x07) {
        case 0x02:
            return ReadStatusRegister();
        case 0x04:
            return ReadOamData();
        case 0x07:
            return ReadDataRegister();
        default:
            return 0x00;
    }
}

void Ppu::WriteRegister(std::uint16_t address, std::uint8_t value) {
    switch (address & 0x07) {
        case 0x00:
            WriteCtrlRegister(value);
            break;
        case 0x01:
            WriteMask(value);
            break;
        case 0x03:
            WriteOamAddr(value);
            break;
        case 0x04:
            WriteOamData(value);
            break;
        case 0x05:
            WriteScroll(value);
            break;
        case 0x06:
            WriteAddr(value);
            break;
        case 0x07:
            WriteData(value);
            break;
        default:
            // PPUSTATUS ($2002) is read-only; writes are ignored.
            break;
    }
}

/// VRAM: the memory router
std::uint8_t Ppu::ReadVram(const std::uint16_t address) const {
    const std::uint16_t addr = address & PpuAddresses::VRAM_MASK;
    if (addr <= PpuAddresses::PATTERN_TABLE_END) {
        return cartridge_.GetMapper().ReadChr(addr);
    }
    if (addr <= PpuAddresses::NAMETABLE_MIRROR_END) {
        return nametable_ram[MirrorNametableAddr(addr)];
    }
    return palette_ram[PaletteIndex(addr)];
}

void Ppu::WriteVram(const std::uint16_t address, const std::uint8_t value) {
    const std::uint16_t addr = address & PpuAddresses::VRAM_MASK;
    if (addr <= PpuAddresses::PATTERN_TABLE_END) {
        cartridge_.GetMapper().WriteChr(addr, value);
        return;
    }
    if (addr <= PpuAddresses::NAMETABLE_MIRROR_END) {
        nametable_ram[MirrorNametableAddr(addr)] = value;
        return;
    }
    palette_ram[PaletteIndex(addr)] = value & PpuAddresses::COLOR_MASK;
}

/// Nametable mirroring
std::uint16_t Ppu::MirrorNametableAddr(const std::uint16_t address) const {
    const std::uint16_t relative = address - PpuAddresses::NAMETABLE_START & PpuAddresses::NAMETABLE_AREA_MASK;
    const std::uint16_t nametable = relative / PpuAddresses::NAMETABLE_SIZE;
    const std::uint16_t offest = relative % PpuAddresses::NAMETABLE_SIZE;
    const auto mirror = cartridge_.GetMirroring();

    const std::uint16_t physical = [&]() -> std::uint16_t {
        switch (mirror) {
            case Cartridge::Mirroring::Vertical:
                return nametable & 1;
            case Cartridge::Mirroring::Horizontal:
                return nametable >> 1;
            // case Cartridge::Mirroring::SingleScreenLower:
            //     return 0;                      // 0,0,0,0
            // case Cartridge::Mirroring::SingleScreenUpper:
            //     return 1;
            default:
                return nametable & 1;
        }
    }();

    return physical * PpuAddresses::NAMETABLE_SIZE + offest;
}

/// Palette mirroring
std::uint16_t Ppu::PaletteIndex(const std::uint16_t address) {
    std::uint16_t index = address & PpuAddresses::PALETTE_MASK;
    if (index >= PpuAddresses::PALETTE_SPRITE_BASE && (index & PpuAddresses::PALETTE_COLOR_MASK) == 0) {
        index -= PpuAddresses::PALETTE_SPRITE_BASE;
    }
    return index;
}

/// Timing
/// Advances one cycle. Every 341 cycles a scanline ends.
void Ppu::Step() {
    AdvanceCycle();
}

void Ppu::TriggerNmi() const {
    if (!nmi_callback_) {
        throw std::runtime_error("nmi_callback_ is null");
    }
    nmi_callback_();
}

void Ppu::AdvanceCycle() {
    cycle_ += 1;
    if (cycle_ >= CYCLES_PER_SCANLINE) {
        cycle_ = 0;
        scanline_ += 1;
        if (scanline_ == VBLANK_SCANLINE) {
            SetVblank();
            frame_complete_ = true;
            if (isNmiEnabled()) TriggerNmi();
        } else if (scanline_ == PRE_RENDER_SCANLINE) {
            ClearVblank();
        } else if (scanline_ >= SCANLINES_PER_FRAME) {
            scanline_ = 0;
        }
    }
}
} // namespace nes