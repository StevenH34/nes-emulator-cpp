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
    /// 341 cycles per scanline
    static constexpr int CYCLES_PER_SCANLINE = 341;
    /// 262 scanlines per frame, 240 visible, 22 vblank, 1 pre-render
    static constexpr int SCANLINES_PER_FRAME = 262;
    /// VBlank scanlines starts a line 241
    static constexpr int VBLANK_SCANLINE = 241;
    /// Pre-render scanline is line 261, the last scanline of the frame
    static constexpr int PRE_RENDER_SCANLINE = 261;
    /// NES PPU has 64 fixed colors
    /// 4 rows of 16 colors:
    ///   Row 0: Dark
    ///   Row 1: Medium
    ///   Row 2: Bright
    ///   Row 3: Pastel
    static constexpr std::array<std::array<int, 3>, 64> PALETTE = {{
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
    /// Clears bits
    static constexpr std::uint16_t CLEAR_NAMETABLE = 0xF3FF;
    static constexpr std::uint16_t CLEAR_COARSE_X  = 0xFFE0;
    /// Clears coarse Y + fine Y, but keeps the nametable and coarse X bits.
    static constexpr std::uint16_t CLEAR_ALL_Y     = 0x0C1F;
    /// Ctrl register constants
    static constexpr std::uint8_t FLAG_VRAM_INCREMENT = 0x04;
    static constexpr std::uint8_t FLAG_NMI_ENABLED    = 0x80;
    /// Status register constant
    static constexpr std::uint8_t FLAG_VBLANK = 0x08;
    /// Scroll register constant
    static constexpr std::uint8_t FINE_BITS = 0x07;

    /// Internal register accessors (for tests; v/t/x have no other observable read path)
    [[nodiscard]] std::uint16_t GetV() const { return v_register_; }
    [[nodiscard]] std::uint16_t GetT() const { return t_register_; }
    [[nodiscard]] std::uint8_t GetX() const { return x_register_; }

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

    /// Mask register method
    /// PPUMASK controls what the PPU draws
    void WriteMask(const std::uint8_t value) { mask_register_ = value; }

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
    [[nodiscard]] std::uint8_t ReadOamData() const { return oam[oam_addr_register_]; }
    void WriteOamData(std::uint8_t value);

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
    [[nodiscard]] bool IsFrameComplete() const { return frame_complete_; }
    void ClearFrameComplete() { frame_complete_ = false; }
    void TriggerNmi() const;
    void AdvanceCycle();


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
    std::vector<std::uint8_t> nametable_ram = std::vector<std::uint8_t>(PpuAddresses::NAMETABLE_RAM_SIZE, 0);
    std::vector<std::uint8_t> palette_ram = std::vector<std::uint8_t>(PpuAddresses::PALETTE_RAM_SIZE, 0);
    std::vector<std::uint8_t> oam = std::vector<std::uint8_t>(PpuAddresses::OAM_SIZE, 0);
    std::uint8_t ctrl_register_{0};
    std::uint8_t mask_register_{0};
    std::uint8_t status_register_{0};
    std::uint8_t oam_addr_register_{0};

    /// Internal PPU Registers
    /// V (15 bits): Scroll position during rendering. Holds VRAM address during VBlank.
    /// T (15 bits): Specifies the starting coarse-x scroll for the next scanline and the starting y scroll for the screen.
    ///     Holds the scroll or VRAM address before transferring it to v during VBlank.
    /// X (3 bits): the pixel offest within a tile.
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
