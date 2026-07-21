#include "doctest.h"

#include "../src/core/ppu/Ppu.h"
#include "../src/core/ppu/Ppu_Addresses.h"
#include "../src/core/Cartridge.h"
#include "TestRom.h"

#include <vector>

namespace {

std::vector<std::uint8_t> MakeHeader(const std::uint8_t prg_blocks, const std::uint8_t chr_blocks,
                                      const std::uint8_t flags_6, const std::uint8_t flags_7) {
    return {0x4E, 0x45, 0x53, 0x1A, prg_blocks, chr_blocks, flags_6, flags_7,
            0, 0, 0, 0, 0, 0, 0, 0};
}

// Builds a mapper-0 cartridge with the given CHR-ROM bytes (padded to 8KB) and mirroring flag,
// so PPU tests can rely on known CHR content and a chosen mirroring mode.
nes::Cartridge MakeCartridge(std::vector<std::uint8_t> chr_bytes = {}, const std::uint8_t flags_6 = 0) {
    auto data = MakeHeader(1, 1, flags_6, 0);
    data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE, 0);
    chr_bytes.resize(nes::Cartridge::CHR_BLOCK_SIZE, 0);
    data.insert(data.end(), chr_bytes.begin(), chr_bytes.end());
    const nes_test::TempRomFile rom(data);
    return nes::Cartridge(rom.path());
}

// Mimics a CPU writing PPUADDR ($2006) twice: high byte first, then low byte.
void SetVAddress(nes::Ppu& ppu, const std::uint16_t address) {
    ppu.WriteAddr(static_cast<std::uint8_t>(address >> 8));
    ppu.WriteAddr(static_cast<std::uint8_t>(address & 0xFF));
}

void StepN(nes::Ppu& ppu, const int cycles) {
    for (int i = 0; i < cycles; ++i) {
        ppu.Step();
    }
}

}

// --- Ctrl register ---

TEST_CASE("VramIncrement is 1 when the VRAM increment flag is clear") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    CHECK(ppu.VramIncrement() == 1);
}

TEST_CASE("VramIncrement is 32 when the VRAM increment flag is set") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    ppu.WriteCtrlRegister(nes::Ppu::FLAG_VRAM_INCREMENT);
    CHECK(ppu.VramIncrement() == 32);
}

TEST_CASE("isNmiEnabled reflects bit 7 of the ctrl register") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    CHECK_FALSE(ppu.isNmiEnabled());
    ppu.WriteCtrlRegister(nes::Ppu::FLAG_NMI_ENABLED);
    CHECK(ppu.isNmiEnabled());
}

// --- Internal address/scroll registers (t, v, x) ---

TEST_CASE("SetCoarseX writes coarse X into bits 4-0 of t") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    ppu.SetCoarseX(0x1F);
    CHECK((ppu.GetT() & nes::Ppu::MASK_COARSE_X) == 0x1F);
}

TEST_CASE("SetCoarseX does not disturb the other bits of t") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    ppu.SetNametable(3);
    ppu.SetScrollY(7, 0x1F);

    ppu.SetCoarseX(0x00);

    CHECK((ppu.GetT() & nes::Ppu::MASK_NAMETABLE) == 3u << 10);
    CHECK((ppu.GetT() & nes::Ppu::MASK_FINE_Y) == 7u << 12);
    CHECK((ppu.GetT() & nes::Ppu::MASK_COARSE_Y) == 0x1Fu << 5);
}

TEST_CASE("SetNametable writes the nametable selection into bits 11-10 of t") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    ppu.SetNametable(2);
    CHECK((ppu.GetT() & nes::Ppu::MASK_NAMETABLE) == 2u << 10);
}

TEST_CASE("SetScrollY writes fine Y and coarse Y into t") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    ppu.SetScrollY(5, 0x1B);
    CHECK((ppu.GetT() & nes::Ppu::MASK_FINE_Y) == 5u << 12);
    CHECK((ppu.GetT() & nes::Ppu::MASK_COARSE_Y) == 0x1Bu << 5);
}

TEST_CASE("WriteScrollX sets the fine-X register and the coarse-X bits of t") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    ppu.WriteScrollX(0x2B); // 0b00101011 -> fine x = 0b011 = 3, coarse x = 0b00101 = 5
    CHECK(ppu.GetX() == 0x03);
    CHECK((ppu.GetT() & nes::Ppu::MASK_COARSE_X) == 0x05);
}

TEST_CASE("WriteScrollY sets fine Y and coarse Y bits of t") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    ppu.WriteScrollY(0x2B); // fine y = 3, coarse y = 5
    CHECK((ppu.GetT() & nes::Ppu::MASK_FINE_Y) == 3u << 12);
    CHECK((ppu.GetT() & nes::Ppu::MASK_COARSE_Y) == 5u << 5);
}

TEST_CASE("WriteScroll's first write sets the X scroll and the second sets the Y scroll") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);

    ppu.WriteScroll(0x08); // first write -> X: fine_x = 0, coarse_x = 1
    CHECK((ppu.GetT() & nes::Ppu::MASK_COARSE_X) == 1);
    CHECK((ppu.GetT() & nes::Ppu::MASK_COARSE_Y) == 0);

    ppu.WriteScroll(0x10); // second write -> Y: fine_y = 0, coarse_y = 2
    CHECK((ppu.GetT() & nes::Ppu::MASK_COARSE_X) == 1); // untouched by the Y write
    CHECK((ppu.GetT() & nes::Ppu::MASK_COARSE_Y) == 2u << 5);
}

TEST_CASE("WriteAddr's first write sets t's high byte without touching v") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    ppu.WriteAddr(0x21);
    CHECK(ppu.GetT() == 0x2100);
    CHECK(ppu.GetV() == 0x0000);
}

TEST_CASE("WriteAddr's second write sets t's low byte and copies t into v") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    ppu.WriteAddr(0x21);
    ppu.WriteAddr(0x05);
    CHECK(ppu.GetT() == 0x2105);
    CHECK(ppu.GetV() == 0x2105);
}

TEST_CASE("WriteAddr masks the first write's high byte to 6 bits (14-bit address space)") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    ppu.WriteAddr(0xFF); // top 2 bits should be dropped
    CHECK(ppu.GetT() == 0x3F00);
}

TEST_CASE("IncrementVRegister wraps v within the 14-bit VRAM address space") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    ppu.WriteAddr(0x3F);
    ppu.WriteAddr(0xFF); // v = 0x3FFF
    ppu.WriteCtrlRegister(nes::Ppu::FLAG_VRAM_INCREMENT); // +32

    ppu.IncrementVRegister();

    CHECK(ppu.GetV() == 0x001F); // (0x3FFF + 32) & 0x3FFF
}

// --- Status register ---

TEST_CASE("SetVblank and ClearVblank toggle the VBlank flag") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);

    ppu.SetVblank();
    CHECK((ppu.ReadStatusRegister() & nes::Ppu::FLAG_VBLANK) != 0);

    ppu.SetVblank();
    ppu.ClearVblank();
    CHECK((ppu.ReadStatusRegister() & nes::Ppu::FLAG_VBLANK) == 0);
}

TEST_CASE("ReadStatusRegister clears the VBlank flag as a side effect") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    ppu.SetVblank();

    const std::uint8_t first = ppu.ReadStatusRegister();
    const std::uint8_t second = ppu.ReadStatusRegister();

    CHECK((first & nes::Ppu::FLAG_VBLANK) != 0);
    CHECK((second & nes::Ppu::FLAG_VBLANK) == 0);
}

TEST_CASE("ReadStatusRegister resets the write latch") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    ppu.WriteAddr(0x20);
    REQUIRE(ppu.IsLatchOn());

    ppu.ReadStatusRegister();

    CHECK_FALSE(ppu.IsLatchOn());
}

// --- Write latch (shared by PPUSCROLL and PPUADDR) ---

TEST_CASE("WriteScroll toggles the latch on each write") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);

    CHECK_FALSE(ppu.IsLatchOn());
    ppu.WriteScroll(0x00);
    CHECK(ppu.IsLatchOn());
    ppu.WriteScroll(0x00);
    CHECK_FALSE(ppu.IsLatchOn());
}

TEST_CASE("WriteAddr toggles the latch on each write") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);

    CHECK_FALSE(ppu.IsLatchOn());
    ppu.WriteAddr(0x20);
    CHECK(ppu.IsLatchOn());
    ppu.WriteAddr(0x00);
    CHECK_FALSE(ppu.IsLatchOn());
}

TEST_CASE("PPUSCROLL and PPUADDR writes share the same latch") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);

    ppu.WriteScroll(0x00);
    CHECK(ppu.IsLatchOn());
    ppu.WriteAddr(0x00);
    CHECK_FALSE(ppu.IsLatchOn());
}

// --- PPUADDR / PPUDATA ---

TEST_CASE("WriteAddr sets v from two writes, high byte first") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);

    SetVAddress(ppu, 0x2005);
    ppu.WriteData(0xAB);

    CHECK(ppu.ReadVram(0x2005) == 0xAB);
}

TEST_CASE("WriteData advances v by 1 when the VRAM increment flag is clear") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);

    SetVAddress(ppu, 0x2000);
    ppu.WriteData(0x11);
    ppu.WriteData(0x22);

    CHECK(ppu.ReadVram(0x2000) == 0x11);
    CHECK(ppu.ReadVram(0x2001) == 0x22);
}

TEST_CASE("WriteData advances v by 32 when the VRAM increment flag is set") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    ppu.WriteCtrlRegister(nes::Ppu::FLAG_VRAM_INCREMENT);

    SetVAddress(ppu, 0x2000);
    ppu.WriteData(0x11);
    ppu.WriteData(0x22);

    CHECK(ppu.ReadVram(0x2000) == 0x11);
    CHECK(ppu.ReadVram(0x2001) == 0x00);
    CHECK(ppu.ReadVram(0x2020) == 0x22);
}

TEST_CASE("ReadDataRegister buffers reads below the palette range by one read") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    SetVAddress(ppu, 0x2010);
    ppu.WriteData(0xAB);

    SetVAddress(ppu, 0x2010);
    const std::uint8_t first = ppu.ReadDataRegister();
    const std::uint8_t second = ppu.ReadDataRegister();

    CHECK(first == 0x00);   // stale buffer from before the address was set
    CHECK(second == 0xAB);  // buffer filled by the first read
}

TEST_CASE("ReadDataRegister returns palette reads immediately, without buffering") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    SetVAddress(ppu, 0x3F00);
    ppu.WriteData(0x24);

    SetVAddress(ppu, 0x3F00);
    const std::uint8_t result = ppu.ReadDataRegister();

    CHECK(result == 0x24);
}

TEST_CASE("ReadDataRegister on a palette address refills the buffer from 0x1000 below") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    SetVAddress(ppu, 0x2F00);
    ppu.WriteData(0x77); // seeds the nametable mirror that 0x3F00 - 0x1000 resolves to

    SetVAddress(ppu, 0x3F00);
    ppu.ReadDataRegister(); // palette read: buffer <- ReadVram(0x2F00)

    SetVAddress(ppu, 0x2123); // any non-palette address; read returns the stale buffer first
    const std::uint8_t buffered = ppu.ReadDataRegister();

    CHECK(buffered == 0x77);
}

// --- OAMADDR / OAMDATA ---

TEST_CASE("WriteOamAddr sets the address used by ReadOamData/WriteOamData") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);

    ppu.WriteOamAddr(0x10);
    ppu.WriteOamData(0x55);
    ppu.WriteOamAddr(0x10);

    CHECK(ppu.ReadOamData() == 0x55);
}

TEST_CASE("WriteOamData increments the OAM address after each write") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);

    ppu.WriteOamAddr(0x00);
    ppu.WriteOamData(0xAA);
    ppu.WriteOamData(0xBB);

    ppu.WriteOamAddr(0x00);
    CHECK(ppu.ReadOamData() == 0xAA);
    ppu.WriteOamAddr(0x01);
    CHECK(ppu.ReadOamData() == 0xBB);
}

TEST_CASE("WriteOamData wraps the OAM address at 256") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);

    ppu.WriteOamAddr(0xFF);
    ppu.WriteOamData(0x11); // oam[0xFF] = 0x11, address wraps to 0x00
    ppu.WriteOamData(0x22); // oam[0x00] = 0x22

    ppu.WriteOamAddr(0x00);
    CHECK(ppu.ReadOamData() == 0x22);
    ppu.WriteOamAddr(0xFF);
    CHECK(ppu.ReadOamData() == 0x11);
}

// --- Register router ---

TEST_CASE("WriteRegister routes $2000 to the ctrl register") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    ppu.WriteRegister(0x2000, nes::Ppu::FLAG_VRAM_INCREMENT);
    CHECK(ppu.VramIncrement() == 32);
}

TEST_CASE("WriteRegister routes $2003/$2004 to OAMADDR/OAMDATA") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);

    ppu.WriteRegister(0x2003, 0x05);
    ppu.WriteRegister(0x2004, 0x9A);
    ppu.WriteRegister(0x2003, 0x05);

    CHECK(ppu.ReadRegister(0x2004) == 0x9A);
}

TEST_CASE("WriteRegister routes $2005 and $2006 through the shared write latch") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);

    CHECK_FALSE(ppu.IsLatchOn());
    ppu.WriteRegister(0x2005, 0x00);
    CHECK(ppu.IsLatchOn());
    ppu.WriteRegister(0x2006, 0x00);
    CHECK_FALSE(ppu.IsLatchOn());
}

TEST_CASE("WriteRegister and ReadRegister route $2007 through PPUDATA") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);

    ppu.WriteRegister(0x2006, 0x20);
    ppu.WriteRegister(0x2006, 0x10);
    ppu.WriteRegister(0x2007, 0x5A);

    CHECK(ppu.ReadVram(0x2010) == 0x5A);
}

TEST_CASE("ReadRegister routes $2002 to the status register and clears VBlank") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    ppu.SetVblank();

    CHECK((ppu.ReadRegister(0x2002) & nes::Ppu::FLAG_VBLANK) != 0);
    CHECK((ppu.ReadRegister(0x2002) & nes::Ppu::FLAG_VBLANK) == 0);
}

TEST_CASE("WriteRegister ignores writes to $2002, the read-only status register") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    ppu.WriteRegister(0x2002, 0xFF);
    CHECK(ppu.ReadStatusRegister() == 0x00);
}

TEST_CASE("ReadRegister returns 0 for write-only register offsets") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    CHECK(ppu.ReadRegister(0x2000) == 0x00);
    CHECK(ppu.ReadRegister(0x2001) == 0x00);
    CHECK(ppu.ReadRegister(0x2003) == 0x00);
    CHECK(ppu.ReadRegister(0x2005) == 0x00);
    CHECK(ppu.ReadRegister(0x2006) == 0x00);
}

TEST_CASE("The register router mirrors every 8 bytes") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    ppu.WriteRegister(0x2008, nes::Ppu::FLAG_VRAM_INCREMENT); // 0x2008 & 7 == 0, same as $2000
    CHECK(ppu.VramIncrement() == 32);
}

// --- VRAM routing ---

TEST_CASE("ReadVram routes pattern-table addresses to the cartridge's CHR-ROM") {
    std::vector<std::uint8_t> chr(nes::Cartridge::CHR_BLOCK_SIZE, 0);
    chr.front() = 0x11;
    chr.back() = 0x22;
    auto cart = MakeCartridge(chr);
    nes::Ppu ppu(cart);

    CHECK(ppu.ReadVram(0x0000) == 0x11);
    CHECK(ppu.ReadVram(0x1FFF) == 0x22);
}

TEST_CASE("WriteVram/ReadVram share physical nametable RAM across mirrored addresses") {
    auto cart = MakeCartridge(); // horizontal mirroring (flags_6 = 0)
    nes::Ppu ppu(cart);

    ppu.WriteVram(0x2000, 0x5A);

    CHECK(ppu.ReadVram(0x2400) == 0x5A);
}

TEST_CASE("ReadVram mirrors $3000-$3EFF down to $2000-$2EFF") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);

    ppu.WriteVram(0x2000, 0x77);

    CHECK(ppu.ReadVram(0x3000) == 0x77);
}

TEST_CASE("MirrorNametableAddr treats nametables 0 and 1 identically under horizontal mirroring") {
    auto cart = MakeCartridge({}, 0); // Horizontal
    nes::Ppu ppu(cart);

    CHECK(ppu.MirrorNametableAddr(0x2000) == ppu.MirrorNametableAddr(0x2400));
    CHECK(ppu.MirrorNametableAddr(0x2000) != ppu.MirrorNametableAddr(0x2800));
}

TEST_CASE("MirrorNametableAddr treats nametables 0 and 2 identically under vertical mirroring") {
    auto cart = MakeCartridge({}, nes::Cartridge::MIRROR_MASK); // Vertical
    nes::Ppu ppu(cart);

    CHECK(ppu.MirrorNametableAddr(0x2000) == ppu.MirrorNametableAddr(0x2800));
    CHECK(ppu.MirrorNametableAddr(0x2000) != ppu.MirrorNametableAddr(0x2400));
}

// --- Palette ---

TEST_CASE("PaletteIndex maps a base color address directly") {
    CHECK(nes::Ppu::PaletteIndex(0x3F00) == 0x00);
    CHECK(nes::Ppu::PaletteIndex(0x3F11) == 0x11);
}

TEST_CASE("PaletteIndex mirrors sprite palette 'color 0' entries into the background palette") {
    CHECK(nes::Ppu::PaletteIndex(0x3F10) == 0x00);
    CHECK(nes::Ppu::PaletteIndex(0x3F14) == 0x04);
}

TEST_CASE("PaletteIndex does not mirror non-zero sprite palette colors") {
    CHECK(nes::Ppu::PaletteIndex(0x3F1D) == 0x1D);
}

TEST_CASE("WriteVram masks palette writes to 6 bits") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    ppu.WriteVram(0x3F00, 0xFF);
    CHECK(ppu.ReadVram(0x3F00) == nes::PpuAddresses::COLOR_MASK);
}

TEST_CASE("WriteVram/ReadVram share palette RAM across the sprite/background mirror") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    ppu.WriteVram(0x3F00, 0x10);
    CHECK(ppu.ReadVram(0x3F10) == 0x10);
}

// --- Timing ---

TEST_CASE("Step does not set VBlank before scanline 241 is reached") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    StepN(ppu, nes::Ppu::CYCLES_PER_SCANLINE * nes::Ppu::VBLANK_SCANLINE - 1);

    CHECK_FALSE(ppu.IsFrameComplete());
    CHECK((ppu.ReadStatusRegister() & nes::Ppu::FLAG_VBLANK) == 0);
}

TEST_CASE("Step sets VBlank and marks the frame complete at scanline 241") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    StepN(ppu, nes::Ppu::CYCLES_PER_SCANLINE * nes::Ppu::VBLANK_SCANLINE);

    CHECK(ppu.IsFrameComplete());
    CHECK((ppu.ReadStatusRegister() & nes::Ppu::FLAG_VBLANK) != 0);
}

TEST_CASE("Step clears VBlank at the pre-render scanline") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    StepN(ppu, nes::Ppu::CYCLES_PER_SCANLINE * nes::Ppu::PRE_RENDER_SCANLINE);

    CHECK((ppu.ReadStatusRegister() & nes::Ppu::FLAG_VBLANK) == 0);
}

TEST_CASE("ClearFrameComplete resets the frame-complete flag") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    StepN(ppu, nes::Ppu::CYCLES_PER_SCANLINE * nes::Ppu::VBLANK_SCANLINE);
    REQUIRE(ppu.IsFrameComplete());

    ppu.ClearFrameComplete();

    CHECK_FALSE(ppu.IsFrameComplete());
}

TEST_CASE("NMI callback fires exactly once per frame when NMI is enabled") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    int nmi_count = 0;
    ppu.SetNmiCallback([&nmi_count] { nmi_count += 1; });
    ppu.WriteCtrlRegister(nes::Ppu::FLAG_NMI_ENABLED);

    StepN(ppu, nes::Ppu::CYCLES_PER_SCANLINE * nes::Ppu::SCANLINES_PER_FRAME);

    CHECK(nmi_count == 1);
}

TEST_CASE("NMI callback does not fire when NMI is disabled") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    int nmi_count = 0;
    ppu.SetNmiCallback([&nmi_count] { nmi_count += 1; });

    StepN(ppu, nes::Ppu::CYCLES_PER_SCANLINE * nes::Ppu::SCANLINES_PER_FRAME);

    CHECK(nmi_count == 0);
}

TEST_CASE("Step throws when NMI is enabled at VBlank but no callback is registered") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    ppu.WriteCtrlRegister(nes::Ppu::FLAG_NMI_ENABLED);
    StepN(ppu, nes::Ppu::CYCLES_PER_SCANLINE * nes::Ppu::VBLANK_SCANLINE - 1);

    CHECK_THROWS_AS(ppu.Step(), std::runtime_error);
}

TEST_CASE("TriggerNmi throws when no NMI callback has been registered") {
    auto cart = MakeCartridge();
    nes::Ppu ppu(cart);
    CHECK_THROWS_AS(ppu.TriggerNmi(), std::runtime_error);
}
