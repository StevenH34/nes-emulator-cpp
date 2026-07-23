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

void Ppu::TriggerNmi() const {
    if (!nmi_callback_) {
        throw std::runtime_error("nmi_callback_ is null");
    }
    nmi_callback_();
}

/// Timing
/// Advances one cycle. Every 341 cycles a scanline ends.
void Ppu::Step() {
    RenderIfVisible();
    UpdateScrollRegisters();
    AdvanceCycle();
}

/// Checks if we're on a visible scanline (between 0 and 239) and if we're at the start of a new scanline (cycle 0).
/// Actual NES hardware renders each pixel across the scanline, but for simplicity we render the entire scanline at once.
void Ppu::RenderIfVisible() {
    if (scanline_ < HEIGHT && cycle_ == 0) {
        RenderScanline(scanline_);
    }
}

/// If rendering is enabled, and we're on a visible scanline, update the scroll registers.
void Ppu::UpdateScrollRegisters() {
    if (!IsRenderingEnabled()) return;
    if (scanline_ >= HEIGHT && scanline_ != PRE_RENDER_SCANLINE) return;

    if (cycle_ == DOT_FINE_Y_INCREMENT) {
        // Cycle 256: after the row advance one pixel row down
        FineYIncrement();
    } else if (cycle_ == DOT_COPY_HORIZONTAL) {
        // Cycle 257: reset the horizontal position from t register inbetween scanline.
        // This ensures each scanline will start from the same X column.
        CopyHorizontal();
    } else if (scanline_ == PRE_RENDER_SCANLINE && cycle_ >= DOT_COPY_VERTICAL_START && cycle_ <= DOT_COPY_VERTICAL_END) {
        // Cycles 280-304: reset the vertical position from t register after the frame.
        // This will ensure the next frame will start from the same Y row.
        CopyVertical();
    }
}

void Ppu::FineYIncrement() {
    if (GetFineY() < MAX_FINE_Y) {
        v_register_ = v_register_ + FINE_Y_UNIT;
        return;
    }
    v_register_ = v_register_ & ~MASK_FINE_Y; // Clear fine Y
    auto y = GetCoarseY();
    if (y == MAX_COARSE_Y) {
        v_register_ = v_register_ & ~MASK_COARSE_Y ^ FLIP_NAMETABLE_V;
    } else if (y == 31) {
        v_register_ = v_register_ & ~MASK_COARSE_Y;
    } else {
        v_register_ = v_register_ & ~MASK_COARSE_Y | (y + 0x0001) << 5;
    }
}

void Ppu::CopyHorizontal() {
    v_register_ = v_register_ & ~MASK_HORIZONTAL | t_register_ & MASK_HORIZONTAL;
}

void Ppu::CopyVertical() {
    v_register_ = v_register_ & ~MASK_VERTICAL | t_register_ & MASK_VERTICAL;
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

/// The main rendering logic loop.
/// Walks the scanline one tile at a time (not one pixel at a time): everything that's
/// constant for the whole scanline (coarse Y, fine Y, base coarse X/nametable) is read
/// once up front, and everything that's constant per-tile (bitplanes, palette) is fetched
/// once per 8 pixels via FetchBackgroundTile rather than once per pixel.
void Ppu::RenderScanline(const std::int32_t y) {
    if (!IsShowBackground()) {
        for (int pixel = 0; pixel < WIDTH; ++pixel) {
            SetPixel(pixel, y, PaletteColor(0, 0));
        }
        return;
    }

    const int fine_y = GetFineY();
    const int coarse_y = GetCoarseY();
    const int base_coarse_x = GetCoarseX();
    const int base_nametable = GetNametable();

    int pixel = 0;
    while (pixel < WIDTH) {
        // Get X pos of the tile we land on
        const int scroll_x = x_register_ + pixel;
        int tile_column = base_coarse_x + scroll_x / PIXELS_PER_TILE;
        int nametable = base_nametable;

        // A nametable has 32 columns (tiles 0-31)
        // If we go over 32 columns we need to subtract 32 to stay within range
        // The low bit of the nametable needs to be XOR by 1 to switch nametables
        if (tile_column >= TILES_PER_ROW) {
            tile_column -= TILES_PER_ROW;
            nametable ^= 1; // Switch horizontal nametable
        }

        const BackgroundTile tile = FetchBackgroundTile(tile_column, nametable, coarse_y, fine_y);

        // Draw every pixel that falls within this tile before moving to the next one.
        for (int pixel_in_tile = scroll_x % PIXELS_PER_TILE; pixel_in_tile < PIXELS_PER_TILE && pixel < WIDTH; ++pixel_in_tile, ++pixel) {
            const auto [color, palette] = ExtractBackgroundPixel(tile, pixel_in_tile);
            SetPixel(pixel, y, PaletteColor(palette, color));
        }
    }
}

/// Fetches the tile index, both pattern-table bitplanes, and the resolved palette for one
/// tile. This is the only place background rendering touches VRAM, and it happens once per
/// tile rather than once per pixel.
Ppu::BackgroundTile Ppu::FetchBackgroundTile(const int tile_column, const int nametable, const int coarse_y, const int fine_y) const {
    // The nametable starts at $2000 + nametable * 1024 bytes
    // Each row stores 32 bytes
    // The tile's position is: row * 32 + column
    const int nametable_address = PpuAddresses::NAMETABLE_START + nametable * PpuAddresses::NAMETABLE_SIZE;
    const int tile_address = nametable_address + coarse_y * TILES_PER_ROW + tile_column;
    // Read the byte from VRAM to get the tile index in the pattern table
    const int tile_index = ReadVram(static_cast<std::uint16_t>(tile_address));

    // Each tile is 16 bytes in the pattern table
    // 8 bytes for the low bitplane, 8 bytes for the high bitplane.
    // Each byte is one row of the 8 pixel tile.
    const int bitplane_address = static_cast<std::int32_t>(BackgroundPatternTable()) + tile_index * BYTES_PER_TILE + fine_y;
    const std::uint8_t low_bitplane = ReadVram(static_cast<std::uint16_t>(bitplane_address));
    const std::uint8_t high_bitplane = ReadVram(static_cast<std::uint16_t>(bitplane_address + BITPLANE_OFFSET));

    const int palette = TilePalette(nametable_address, tile_column, coarse_y);

    return {low_bitplane, high_bitplane, palette};
}

/// Reads one pixel out of an already-fetched tile's bitplanes. Pure bit math, no VRAM access.
Ppu::Pixel Ppu::ExtractBackgroundPixel(const BackgroundTile& tile, const int pixel_in_tile) {
    const int bit = (PIXELS_PER_TILE - 1) - pixel_in_tile;
    const int low_bit = (tile.low_bitplane >> bit) & 1;
    const int high_bit = (tile.high_bitplane >> bit) & 1;
    // 0 means the pixel is transparent and the tile's palette doesn't matter.
    const int color = (high_bit << 1) | low_bit;
    const int palette = color != 0 ? tile.palette : 0;
    return {color, palette};
}

/// The attribute table
std::int32_t Ppu::TilePalette(const std::int32_t nametable_address, const std::int32_t tile_column, const std::int32_t tile_row) const {
    // Each byte in the attribute table corresponds to a 4x4 block of tiles.
    // Subdivided into 2x2 blocks of tiles, each block uses 2 bits to select a palette.
    const int attribute_column = tile_column / 4;
    const int attribute_row = tile_row / 4;
    const int attribute_address = nametable_address + ATTRIBUTE_TABLE_OFFSET + attribute_row * (TILES_PER_ROW / 4) + attribute_column;
    const std::uint8_t attribute_byte = ReadVram(static_cast<std::uint16_t>(attribute_address));

    const int right = (tile_column / 2) & 1;
    const int bottom = (tile_row / 2) & 1;
    const int shift = (bottom * 2 + right) * 2;

    return (attribute_byte >> shift) & (COLORS_PER_PALETTE - 1);
}

/// The final color
/// Returns an index form 0-63 from the NES master palette.
std::uint8_t Ppu::PaletteColor(const std::int32_t palette, const std::int32_t color) const {
    // If color is 0 this means it's transparent.
    if (color == 0) {
        return ReadVram(PpuAddresses::PALETTE_START);
    }
    return ReadVram(static_cast<std::uint16_t>(PpuAddresses::PALETTE_START + palette * COLORS_PER_PALETTE + color));
}

/// Set pixel to the frame buffer.
void Ppu::SetPixel(const std::int32_t x, const std::int32_t y, const std::uint8_t palette_index) {
    // Look up the RGB color in the master palette.
    const auto [r, g, b] = GetColor(palette_index);
    const int offset = (y * WIDTH + x) * BYTES_PER_PIXEL;
    // Write to frame buffer in RGBA format.
    frame_buffer_[offset] = r;
    frame_buffer_[offset + 1] = g;
    frame_buffer_[offset + 2] = b;
    frame_buffer_[offset + 3] = 0xFF; // Alpha channel - opaque
}

} // namespace nes