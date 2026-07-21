#include "doctest.h"

#include <vector>

#include "../src/core/Bus.h"
#include "../src/core/Cartridge.h"
#include "../src/core/ppu/Ppu.h"
#include "TestBus.h"
#include "TestRom.h"

TEST_CASE("Bus RAM reads and writes are mirrored across the 2 KB window") {
    nes_test::TestBus bus;

    bus.WriteCpu(0x07FF, 0xAB);

    CHECK(bus.ReadCpu(0x07FF) == 0xAB);
    CHECK(bus.ReadCpu(0x0FFF) == 0xAB);
    CHECK(bus.ReadCpu(0x17FF) == 0xAB);
    CHECK(bus.ReadCpu(0x1FFF) == 0xAB);
}

TEST_CASE("Bus RAM read and write helpers access the mirrored backing store") {
    nes_test::TestBus bus;

    bus.WriteRam(0x0000, 0x11);
    bus.WriteRam(0x0800, 0x22);

    CHECK(bus.ReadRam(0x0000) == 0x22);
    CHECK(bus.ReadRam(0x0800) == 0x22);
    CHECK(bus.ReadRam(0x1000) == 0x22);
    CHECK(bus.ReadRam(0x1800) == 0x22);
}

TEST_CASE("Bus RAM starts zero-initialized") {
    nes_test::TestBus bus;

    CHECK(bus.ReadCpu(0x0000) == 0x00);
    CHECK(bus.ReadCpu(0x07FF) == 0x00);
}

TEST_CASE("Bus writes do not bleed into unrelated, non-mirrored addresses") {
    nes_test::TestBus bus;

    bus.WriteCpu(0x0010, 0x99);

    CHECK(bus.ReadCpu(0x0010) == 0x99);
    CHECK(bus.ReadCpu(0x0011) == 0x00);
    CHECK(bus.ReadCpu(0x0800) == 0x00); // mirror of 0x0000, not 0x0010
}

TEST_CASE("Bus returns zero for the unmapped region between RAM and PRG-ROM") {
    nes_test::TestBus bus;

    // 0x2000-0x7FFF is PPU/APU/expansion/SRAM space, none of which Bus wires up yet.
    CHECK(bus.ReadCpu(0x2000) == 0x00);
    CHECK(bus.ReadCpu(0x4000) == 0x00);
    CHECK(bus.ReadCpu(0x7FFF) == 0x00);
}

TEST_CASE("Bus ignores writes to the unmapped region between RAM and PRG-ROM") {
    nes_test::TestBus bus;

    bus.WriteCpu(0x2000, 0xAB);

    CHECK(bus.ReadCpu(0x2000) == 0x00);
    // A naive address-mask without a range check would alias 0x2000 onto 0x0000
    CHECK(bus.ReadCpu(0x0000) == 0x00);
}

TEST_CASE("Bus reads PRG-ROM through the cartridge mapper at 0x8000-0xFFFF") {
    std::vector<std::uint8_t> data{0x4E, 0x45, 0x53, 0x1A, 0x01, 0x01, 0x00, 0x00,
                                    0, 0, 0, 0, 0, 0, 0, 0};
    data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE + nes::Cartridge::CHR_BLOCK_SIZE, 0);
    // Marker byte at PRG offset 0, mirrored by Mapper000's 16 KB mask onto both
    // CPU 0x8000 and 0xC000, so this proves the read comes from the mapper and
    // not from a coincidentally-zeroed backing array.
    data[nes::Cartridge::HEADER_SIZE] = 0x42;

    const nes_test::TempRomFile rom(data);
    nes::Cartridge cartridge(rom.path());
    nes::Ppu ppu(cartridge);
    const nes::Bus bus(cartridge, ppu);

    CHECK(bus.ReadCpu(0x8000) == 0x42);
    CHECK(bus.ReadCpu(0xC000) == 0x42);
}


