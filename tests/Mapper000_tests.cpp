#include "doctest.h"

#include "../src/mappers/Mapper000.h"
#include "../src/Cartridge.h"

#include <vector>

namespace {

std::vector<std::uint8_t> MakePrgRom(const std::size_t size, const std::uint8_t first_byte, const std::uint8_t last_byte) {
    std::vector<std::uint8_t> prg(size, 0);
    prg.front() = first_byte;
    prg.back() = last_byte;
    return prg;
}

}

// --- 16KB PRG-ROM (mirrored) ---

TEST_CASE("Mapper000 mirrors a 16KB PRG-ROM across the full $8000-$FFFF range") {
    const auto prg = MakePrgRom(nes::Cartridge::PRG_BLOCK_SIZE, 0x11, 0x22);
    const nes::Mapper000 mapper(prg, {});

    CHECK(mapper.ReadPrg(0x8000) == mapper.ReadPrg(0xC000));
    CHECK(mapper.ReadPrg(0xBFFF) == mapper.ReadPrg(0xFFFF));
}

TEST_CASE("Mapper000 reads the first and last bytes of a 16KB PRG-ROM") {
    const auto prg = MakePrgRom(nes::Cartridge::PRG_BLOCK_SIZE, 0x11, 0x22);
    const nes::Mapper000 mapper(prg, {});

    CHECK(mapper.ReadPrg(0x8000) == 0x11);
    CHECK(mapper.ReadPrg(0xBFFF) == 0x22);
}

// --- 32KB PRG-ROM (linear) ---

TEST_CASE("Mapper000 does not mirror a 32KB PRG-ROM") {
    const auto prg = MakePrgRom(nes::Cartridge::PRG_BLOCK_SIZE * 2, 0x11, 0x22);
    const nes::Mapper000 mapper(prg, {});

    CHECK(mapper.ReadPrg(0x8000) != mapper.ReadPrg(0xC000));
}

TEST_CASE("Mapper000 reads the first and last bytes of a 32KB PRG-ROM") {
    const auto prg = MakePrgRom(nes::Cartridge::PRG_BLOCK_SIZE * 2, 0x11, 0x22);
    const nes::Mapper000 mapper(prg, {});

    CHECK(mapper.ReadPrg(0x8000) == 0x11);
    CHECK(mapper.ReadPrg(0xFFFF) == 0x22);
}

// --- CHR-ROM ---

TEST_CASE("Mapper000 reads the first and last bytes of an 8KB CHR-ROM") {
    const std::vector<std::uint8_t> prg(nes::Cartridge::PRG_BLOCK_SIZE, 0);
    std::vector<std::uint8_t> chr(nes::Cartridge::CHR_BLOCK_SIZE, 0);
    chr.front() = 0x33;
    chr.back() = 0x44;
    const nes::Mapper000 mapper(prg, chr);

    CHECK(mapper.ReadChr(0x0000) == 0x33);
    CHECK(mapper.ReadChr(0x1FFF) == 0x44);
}