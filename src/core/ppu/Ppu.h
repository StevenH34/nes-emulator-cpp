#ifndef NES_EMULATOR_CPP_PPU_H
#define NES_EMULATOR_CPP_PPU_H

#include <cstdint>
#include <functional>
#include <vector>

#include "Cartridge.h"
#include "Ppu_Addresses.h"

namespace nes {

class Ppu {
public:
    explicit Ppu(Cartridge& cartridge);
    /// Display
    static constexpr int WIDTH  = 256;
    static constexpr int HEIGHT = 240;
    // 341 cycles per scanline
    static constexpr int CYCLES_PER_SCANLINE = 341;
    // 262 scanlines per frame, 240 visible, 22 vblank, 1 pre-render
    static constexpr int SCANLINES_PER_FRAME = 262;
    // VBlank scanlines starts a line 241
    static constexpr int VBLANK_SCANLINE = 241;
    // Pre-render scanline is line 261, the last scanline of the frame
    static constexpr int PRE_RENDER_SCANLINE = 261;
    /// Tile constants
    // 32 tiles per row, 8 pixels per tile, 16 bytes per tile (2 bitplanes)
    static constexpr int TILES_PER_ROW = 32;
    static constexpr int PIXELS_PER_TILE = 8;
    static constexpr int BYTES_PER_TILE = 16;
    static constexpr int BITPLANE_OFFSET = 8;
    // The attribute table starts 960 bytes into each nametable
    static constexpr int ATTRIBUTE_TABLE_OFFSET = 960;
    // 4 colors per palette and 4 bytes per pixel in the frame buffer
    static constexpr int COLORS_PER_PALETTE = 4;
    static constexpr int BYTES_PER_PIXEL = 4;

    struct Color {
        std::uint8_t r, g, b;
    };

    /// NES PPU has 64 fixed colors
    /// 4 rows of 16 colors:
    ///   Row 0: Dark
    ///   Row 1: Medium
    ///   Row 2: Bright
    ///   Row 3: Pastel
    static constexpr std::array<Color, 64> PALETTE = {{
    // $00-$0F: Dark
    {84, 84, 84},   {0, 30, 116},   {8, 16, 144},   {48, 0, 136},
    {68, 0, 100},   {92, 0, 48},    {84, 4, 0},     {60, 24, 0},
    {32, 42, 0},    {8, 58, 0},     {0, 64, 0},     {0, 60, 0},
    {0, 50, 60},    {0, 0, 0},      {0, 0, 0},      {0, 0, 0},

    // $10-$1F: Medium
    {152, 150, 152}, {8, 76, 196},   {48, 50, 236},  {92, 30, 228},
    {136, 20, 176},  {160, 20, 100}, {152, 34, 32},  {120, 60, 0},
    {84, 90, 0},     {40, 114, 0},   {8, 124, 0},    {0, 118, 40},
    {0, 102, 120},   {0, 0, 0},      {0, 0, 0},      {0, 0, 0},

    // $20-$2F: Bright
    {236, 238, 236}, {76, 154, 236},  {120, 124, 236}, {176, 98, 236},
    {228, 84, 236},  {236, 88, 180},  {236, 106, 100}, {212, 136, 32},
    {160, 170, 0},   {116, 196, 0},   {76, 208, 32},   {56, 204, 108},
    {56, 180, 204},  {60, 60, 60},    {0, 0, 0},       {0, 0, 0},

    // $30-$3F: Pastel
    {236, 238, 236}, {168, 204, 236}, {188, 188, 236}, {212, 178, 236},
    {236, 174, 236}, {236, 174, 212}, {236, 180, 176}, {228, 196, 144},
    {204, 210, 120}, {180, 222, 120}, {168, 226, 144}, {152, 226, 180},
    {160, 214, 228}, {160, 162, 160}, {0, 0, 0},       {0, 0, 0},
    }};
    /// Extract bits
    static constexpr std::uint16_t MASK_COARSE_X   = 0x001F;
    static constexpr std::uint16_t MASK_COARSE_Y   = 0x03E0;
    static constexpr std::uint16_t MASK_NAMETABLE  = 0x0C00;
    static constexpr std::uint16_t MASK_FINE_Y     = 0x7000;
    // Horizontal bits: coarse X + horizontal nametable bit
    static constexpr std::uint16_t MASK_HORIZONTAL = 0x041F;
    // Vertical bits: coarse Y + find Y + vertical nametable bit
    static constexpr std::uint16_t MASK_VERTICAL   = 0x07BE0;
    /// Clears bits
    static constexpr std::uint16_t CLEAR_NAMETABLE = 0xF3FF;
    static constexpr std::uint16_t CLEAR_COARSE_X  = 0xFFE0;
    /// Clears coarse Y + fine Y, but keeps the nametable and coarse X bits.
    static constexpr std::uint16_t CLEAR_ALL_Y     = 0x0C1F;
    /// Ctrl register constants
    static constexpr std::uint8_t FLAG_VRAM_INCREMENT = 0x04;
    static constexpr std::uint8_t FLAG_NMI_ENABLED    = 0x80;
    static constexpr std::uint8_t FLAG_SPR_PATTERN_TABLE = 0x08;
    static constexpr std::uint8_t FLAG_BG_PATTERN_TABLE  = 0x10;
    /// Pattern tables
    static constexpr std::uint16_t PATTERN_TABLE_0 = 0x0000;
    static constexpr std::uint16_t PATTERN_TABLE_1 = 0x1000;
    /// Status register constant
    static constexpr std::uint8_t FLAG_VBLANK = 0x80;
    static constexpr std::uint8_t FLAG_SPRITE_0_HIT = 0x40;
    /// Scroll register constant
    static constexpr std::uint8_t FINE_BITS = 0x07;
    /// PPUMASK
    static constexpr std::uint8_t FLAG_SHOW_BG = 0x08;
    static constexpr std::uint8_t FLAG_SHOW_SPRITES = 0x10;
    /// Scanline points of modification
    static constexpr std::int32_t DOT_FINE_Y_INCREMENT = 256;
    static constexpr std::int32_t DOT_COPY_HORIZONTAL  = 257;
    static constexpr std::int32_t DOT_COPY_VERTICAL_START = 280;
    static constexpr std::int32_t DOT_COPY_VERTICAL_END   = 304;
    /// Use to flip the nametable bit in v register via XOR
    static constexpr std::uint16_t FLIP_NAMETABLE_H = 0X0400;
    static constexpr std::uint16_t FLIP_NAMETABLE_V = 0X0800;
    // Add to v register, increments fine y by 1
    static constexpr std::uint16_t FINE_Y_UNIT = 0X1000;
    /// Boundary values
    static constexpr std::int32_t MAX_COARSE_X = 31;
    static constexpr std::int32_t MAX_COARSE_Y = 29;
    static constexpr std::int32_t MAX_FINE_Y = 7;
    /// Sprite constants
    static constexpr std::int32_t SPRITE_Y_OFFSET = 1;
    static constexpr std::uint8_t SPRITE_PALETTE_MASK = 0x03;
    static constexpr std::uint8_t SPRITE_BEHIND_BACKGROUND = 0x20;
    static constexpr std::uint8_t SPRITE_FLIP_H = 0x40;
    static constexpr std::uint8_t SPRITE_FLIP_V = 0x80;
    static constexpr int SPRITE_PALETTE_OFFSET = 4;
    static constexpr int MAX_SPRITES_PER_SCANLINE = 8;
    static constexpr int SPRITES_TOTAL = 64;
    static constexpr int SPRITE_BYTES = 4;

    // Getters
    [[nodiscard]] std::uint16_t GetV() const { return v_register_; }
    [[nodiscard]] std::uint16_t GetT() const { return t_register_; }
    [[nodiscard]] std::uint8_t GetX() const { return x_register_; }
    [[nodiscard]] const std::vector<std::uint8_t>& GetFrameBuffer() const { return frame_buffer_; }
    [[nodiscard]] static constexpr Color GetColor(const std::uint8_t palette_index) { return PALETTE[palette_index & 0x3F]; }

    void SetSprite0Hit() { status_register_ |= FLAG_SPRITE_0_HIT; }
    void ClearSprite0Hit() { status_register_ &= ~FLAG_SPRITE_0_HIT; }

    /// Latch methods
    [[nodiscard]] bool IsLatchOn() const { return w_register_; }
    void ResetLatch() { w_register_ = false; }
    void ToggleLatch() { w_register_ = !w_register_; } // Alternates between first and second write
    void IncrementVRegister();
    void SetCoarseX(std::uint16_t coarse_x);
    void SetNametable(std::uint16_t nametable);
    void SetScrollY(std::uint16_t fine_y, std::uint16_t coarse_y);

    /// Ctrl methods
    void WriteCtrlRegister(std::uint8_t value);
    [[nodiscard]] std::uint16_t VramIncrement() const;
    [[nodiscard]] bool isNmiEnabled() const;
    [[nodiscard]] std::uint16_t BackgroundPatternTable() const;
    [[nodiscard]] std::uint16_t SpritePatternTable() const;

    /// PPUMASK controls what the PPU draws
    void WriteMask(const std::uint8_t value) { mask_register_ = value; }
    /// If bit 3 of PPUMASK is off, background is not drawn
    [[nodiscard]] bool IsShowBackground() const { return (mask_register_ & FLAG_SHOW_BG) != 0; }
    [[nodiscard]] bool IsShowSprites() const { return (mask_register_ & FLAG_SHOW_SPRITES) != 0; }
    /// If either is on, the PPU is rendering
    [[nodiscard]] bool IsRenderingEnabled() const { return IsShowBackground() || IsShowSprites(); }

    /// Status register methods
    std::uint8_t ReadStatusRegister();
    void SetVblank();
    void ClearVblank();

    /// Scroll register methods - PPUSCROLL ($2005)
    void WriteScroll(std::uint8_t value);
    void WriteScrollX(std::uint8_t value);
    void WriteScrollY(std::uint8_t value);

    /// PPUADDR ($2006): VRAM address
    void WriteAddr(std::uint8_t value);

    /// PPUDATA ($2007): VRAM access
    std::uint8_t ReadDataRegister();
    void WriteData(std::uint8_t value);

    /// OAMADDR ($2003): OAM address
    void WriteOamAddr(const std::uint8_t value) {oam_addr_register_ = value;}

    /// OAMDATA ($2004): Sprites
    [[nodiscard]] std::uint8_t ReadOamData() const { return oam_[oam_addr_register_]; }
    void WriteOamData(std::uint8_t value);
    void OamDma(std::array<std::uint8_t, 256> data);

    /// Register router
    std::uint8_t ReadRegister(std::uint16_t address);
    void WriteRegister(std::uint16_t address, std::uint8_t value);

    /// VRAM: the memory router
    [[nodiscard]] std::uint8_t ReadVram(std::uint16_t address) const;
    void WriteVram(std::uint16_t address, std::uint8_t value);
    /// Nametable mirroring
    [[nodiscard]] std::uint16_t MirrorNametableAddr(std::uint16_t address) const;
    /// Palette mirroring
    static std::uint16_t PaletteIndex(std::uint16_t address);

    /// Timing
    void Step();
    void RenderIfVisible();
    void UpdateScrollRegisters();
    [[nodiscard]] bool IsFrameComplete() const { return frame_complete_; }
    void ClearFrameComplete() { frame_complete_ = false; }
    void TriggerNmi() const;
    void AdvanceCycle();
    void SetNmiCallback(std::function<void()> callback) { nmi_callback_ = std::move(callback); }

    /// V register methods
    /// Each getter ANDs with the corresponding mask and shifts down to 0 if necessary
    [[nodiscard]] int GetCoarseX() const { return v_register_ & MASK_COARSE_X; }
    [[nodiscard]] int GetCoarseY() const { return (v_register_ & MASK_COARSE_Y) >> 5; }
    [[nodiscard]] int GetFineY() const { return (v_register_ & MASK_FINE_Y) >> 12; }
    [[nodiscard]] int GetNametable() const { return (v_register_ & MASK_NAMETABLE) >> 10; }

    /// Rendering logic
    struct Pixel { int color; int palette; };
    /// The two bitplane rows and resolved palette for one tile, shared by all 8 pixels
    /// in that tile so VRAM only needs to be read once per tile instead of once per pixel.
    struct BackgroundTile { std::uint8_t low_bitplane; std::uint8_t high_bitplane; int palette; };
    void RenderScanline(std::int32_t y);
    [[nodiscard]] BackgroundTile FetchBackgroundTile(int tile_column, int nametable, int coarse_y, int fine_y) const;
    [[nodiscard]] static int ColorFromBitplanes(std::uint8_t low_bitplane, std::uint8_t high_bitplane, int pixel_in_tile);
    [[nodiscard]] static Pixel ExtractBackgroundPixel(const BackgroundTile& tile, int pixel_in_tile);
    [[nodiscard]] Pixel BackgroundPixelAt(std::int32_t screen_x, std::int32_t y) const;
    [[nodiscard]] std::int32_t TilePalette(std::int32_t nametable_address, std::int32_t tile_column, std::int32_t tile_row) const;
    [[nodiscard]] std::uint8_t PaletteColor(std::int32_t palette, std::int32_t color) const;
    [[nodiscard]] std::uint8_t SpritePaletteColor(std::int32_t palette, std::int32_t color) const;
    [[nodiscard]] std::uint8_t ResolvePaletteColor(std::int32_t palette_group_offset, std::int32_t palette, std::int32_t color) const;
    void SetPixel(std::int32_t x, std::int32_t y, std::uint8_t palette_index);
    [[nodiscard]] std::int32_t SpriteTilePixel(std::uint8_t tile_index, std::int32_t tile_row, std::int32_t pixel_in_tile) const;
    [[nodiscard]] std::tuple<int32_t, int32_t, bool> SpritePixel(std::int32_t x, std::int32_t y) const;


    void FineYIncrement();

    void CopyHorizontal();
    void CopyVertical();

    void CheckSprite0Hit(std::int32_t y);

private:
    /// Will read CHR ROM from Cartridge
    Cartridge& cartridge_;
    std::function<void()> nmi_callback_{nullptr};
    int cycle_{0};
    int scanline_{0};
    bool frame_complete_{false};
    /// @frame_buffer_ 256 x 240 pixels, 4 bytes per pixel (RGBA)
    std::vector<std::uint8_t> frame_buffer_ = std::vector<std::uint8_t>(WIDTH * HEIGHT * 4, 0);
    std::uint8_t vram_buffer_{0};
    std::vector<std::uint8_t> nametable_ram_ = std::vector<std::uint8_t>(PpuAddresses::NAMETABLE_RAM_SIZE, 0);
    std::vector<std::uint8_t> palette_ram_ = std::vector<std::uint8_t>(PpuAddresses::PALETTE_RAM_SIZE, 0);
    std::vector<std::uint8_t> oam_ = std::vector<std::uint8_t>(PpuAddresses::OAM_SIZE, 0);
    std::uint8_t ctrl_register_{0};
    std::uint8_t mask_register_{0};
    std::uint8_t status_register_{0};
    std::uint8_t oam_addr_register_{0};

    /// Internal PPU Registers
    /// V (15 bits): Scroll position during rendering. Holds VRAM address during VBlank.
    /// T (15 bits): Specifies the starting coarse-x scroll for the next scanline and the starting y scroll for the screen.
    ///     Holds the scroll or VRAM address before transferring it to v during VBlank.
    /// X (3 bits): the pixel offset within a tile.
    ///     The fine-x position of the current scroll, used during rendering alongside v.
    /// W (1 bit): Toggles on each write to either PPUSCROLL or PPUADDR, indicating first or second write.
    ///     Clears on reads of PPUSTATUS.
    std::uint16_t v_register_{0};
    std::uint16_t t_register_{0};
    std::uint8_t x_register_{0};
    bool w_register_{false};
};

} // namespace nes

#endif //NES_EMULATOR_CPP_PPU_H
