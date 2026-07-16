#ifndef NES_EMULATOR_CPP_PPU_ADDRESSES_H
#define NES_EMULATOR_CPP_PPU_ADDRESSES_H

#include <cstdint>
#include <cstddef>

namespace nes {

struct PpuAddresses {
    /// PPU memory map
    static constexpr std::uint16_t PATTERN_TABLE_START  = 0x0000;
    static constexpr std::uint16_t PATTERN_TABLE_END    = 0x1FFF;
    static constexpr std::uint16_t NAMETABLE_START      = 0x2000;
    static constexpr std::uint16_t NAMETABLE_MIRROR_END = 0x3EFF;
    static constexpr std::uint16_t PALETTE_START        = 0x3F00;
    static constexpr std::uint16_t VRAM_MASK            = 0x3FFF;

    /// Internal memory sizes
    static constexpr std::size_t NAMETABLE_RAM_SIZE = 2048; // 2KB physical VRAM (holds 2 nametables)
    static constexpr std::size_t PALETTE_RAM_SIZE   = 32;
    static constexpr std::size_t OAM_SIZE           = 256;  // 64 sprites x 4 bytes

    /// Nametable
    static constexpr std::uint16_t NAMETABLE_SIZE      = 1024;   // 1KB per nametable
    static constexpr std::uint16_t NAMETABLE_AREA_MASK = 0x0FFF; // Wraps address within the 4 logical nametables (4KB)

    /// Palette
    static constexpr std::uint16_t PALETTE_MASK        = 0x1F;
    static constexpr std::uint16_t PALETTE_SPRITE_BASE = 0x10; // Sprite palettes start at $3F10
    static constexpr std::uint16_t PALETTE_COLOR_MASK  = 0x03; // Color 0 of each palette (every 4th entry)
    static constexpr std::uint8_t COLOR_MASK           = 0x3F; // NES has 64 colors total
};

} // namespace nes

#endif //NES_EMULATOR_CPP_PPU_ADDRESSES_H
