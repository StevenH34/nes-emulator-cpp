#include "../doctest.h"

#include "../src/Cartridge.h"

#include <atomic>
#include <filesystem>
#include <fstream>
#include <vector>

namespace {

std::vector<std::uint8_t> MakeHeader(const std::uint8_t prg_blocks, const std::uint8_t chr_blocks,
                                      const std::uint8_t flags_6, const std::uint8_t flags_7) {
    return {0x4E, 0x45, 0x53, 0x1A, prg_blocks, chr_blocks, flags_6, flags_7,
            0, 0, 0, 0, 0, 0, 0, 0};
}

// Writes a byte buffer to a uniquely-named temp file and deletes it on destruction,
// so each test gets an isolated .nes file on disk for Cartridge's file-based constructor.
class TempRomFile {
public:
    explicit TempRomFile(const std::vector<std::uint8_t>& data) {
        static std::atomic<int> counter{0};
        path_ = (std::filesystem::temp_directory_path() /
                 ("cartridge_test_" + std::to_string(counter++) + ".nes")).string();
        std::ofstream file(path_, std::ios::binary);
        file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    }

    ~TempRomFile() {
        std::error_code ec;
        std::filesystem::remove(path_, ec);
    }

    TempRomFile(const TempRomFile&) = delete;
    TempRomFile& operator=(const TempRomFile&) = delete;

    [[nodiscard]] const std::string& path() const { return path_; }

private:
    std::string path_;
};

}

// --- ParseMapperId ---

TEST_CASE("ParseMapperId returns 0 when both nibbles are 0") {
    CHECK(nes::Cartridge::ParseMapperId(0x00, 0x00) == 0);
}

TEST_CASE("ParseMapperId reads the low nibble from the top bits of flags_6") {
    CHECK(nes::Cartridge::ParseMapperId(0x10, 0x00) == 0x01);
}

TEST_CASE("ParseMapperId reads the high nibble from the top bits of flags_7") {
    CHECK(nes::Cartridge::ParseMapperId(0x00, 0x10) == 0x10);
}

TEST_CASE("ParseMapperId combines both nibbles into the full mapper number") {
    // flags_6 top nibble = 0xA, flags_7 top nibble = 0x5 -> mapper 0x5A (90)
    CHECK(nes::Cartridge::ParseMapperId(0xA0, 0x50) == 0x5A);
}

TEST_CASE("ParseMapperId ignores the lower flag bits of flags_6 and flags_7") {
    // Lower nibble of flags_6 carries mirroring/battery/trainer bits and must not leak into the mapper id
    CHECK(nes::Cartridge::ParseMapperId(0xA3, 0x5C) == 0x5A);
}

// --- ParseMirroring ---

TEST_CASE("ParseMirroring returns Horizontal when no mirroring bits are set") {
    CHECK(nes::Cartridge::ParseMirroring(0x00) == nes::Cartridge::Mirroring::Horizontal);
}

TEST_CASE("ParseMirroring returns Vertical when the mirror bit is set") {
    CHECK(nes::Cartridge::ParseMirroring(nes::Cartridge::MIRROR_MASK) == nes::Cartridge::Mirroring::Vertical);
}

TEST_CASE("ParseMirroring returns FourScreen when the four-screen bit is set") {
    CHECK(nes::Cartridge::ParseMirroring(nes::Cartridge::FOUR_SCREEN_MASK) == nes::Cartridge::Mirroring::FourScreen);
}

TEST_CASE("ParseMirroring gives FourScreen priority over the mirror bit") {
    const std::uint8_t flags_6 = nes::Cartridge::FOUR_SCREEN_MASK | nes::Cartridge::MIRROR_MASK;
    CHECK(nes::Cartridge::ParseMirroring(flags_6) == nes::Cartridge::Mirroring::FourScreen);
}

// --- ValidateHeader ---

TEST_CASE("ValidateHeader accepts a well-formed header") {
    const auto data = MakeHeader(1, 1, 0, 0);
    CHECK_NOTHROW(nes::Cartridge::ValidateHeader(data));
}

TEST_CASE("ValidateHeader throws when the data is smaller than the header") {
    const std::vector<std::uint8_t> data(10, 0);
    CHECK_THROWS_AS(nes::Cartridge::ValidateHeader(data), std::runtime_error);
}

TEST_CASE("ValidateHeader throws when the magic number is wrong") {
    auto data = MakeHeader(1, 1, 0, 0);
    data[0] = 0x00;
    CHECK_THROWS_AS(nes::Cartridge::ValidateHeader(data), std::runtime_error);
}

// --- ReadFileBytes ---

TEST_CASE("ReadFileBytes returns the exact bytes on disk") {
    const std::vector<std::uint8_t> expected = {0x4E, 0x45, 0x53, 0x1A, 0xDE, 0xAD, 0xBE, 0xEF};
    const TempRomFile rom(expected);

    const auto actual = nes::Cartridge::ReadFileBytes(rom.path());

    CHECK(actual == expected);
}

TEST_CASE("ReadFileBytes throws when the file does not exist") {
    CHECK_THROWS_AS(nes::Cartridge::ReadFileBytes("/no/such/file/definitely_missing.nes"), std::runtime_error);
}

// --- Cartridge construction ---

TEST_CASE("Cartridge parses a minimal one-bank ROM with no trainer") {
    auto data = MakeHeader(1, 1, 0, 0);
    data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE + nes::Cartridge::CHR_BLOCK_SIZE, 0);
    data[16] = 0x11;                                                     // first PRG-ROM byte
    data[16 + nes::Cartridge::PRG_BLOCK_SIZE] = 0x22;                    // first CHR-ROM byte
    const TempRomFile rom(data);

    const nes::Cartridge cart(rom.path());

    CHECK(cart.GetPath() == rom.path());
    CHECK(cart.GetPrgRom().size() == nes::Cartridge::PRG_BLOCK_SIZE);
    CHECK(cart.GetChrRom().size() == nes::Cartridge::CHR_BLOCK_SIZE);
    CHECK(cart.GetPrgRom()[0] == 0x11);
    CHECK(cart.GetChrRom()[0] == 0x22);
    CHECK(cart.GetMirroring() == nes::Cartridge::Mirroring::Horizontal);
    CHECK_FALSE(cart.HasBatteryBackedRam());
}

TEST_CASE("Cartridge synthesizes 8KB of zeroed CHR-RAM when CHR-ROM size is 0") {
    auto data = MakeHeader(1, 0, 0, 0);
    data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE, 0);
    const TempRomFile rom(data);

    const nes::Cartridge cart(rom.path());

    CHECK(cart.GetChrRom().size() == nes::Cartridge::CHR_BLOCK_SIZE);
    CHECK(cart.GetChrRom() == std::vector<std::uint8_t>(nes::Cartridge::CHR_BLOCK_SIZE, 0));
}

TEST_CASE("Cartridge skips the 512-byte trainer before reading PRG-ROM") {
    auto data = MakeHeader(1, 1, nes::Cartridge::TRAINER_MASK, 0);
    data.resize(data.size() + nes::Cartridge::TRAINER_SIZE, 0xFF);       // trainer bytes
    data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE + nes::Cartridge::CHR_BLOCK_SIZE, 0);
    const std::size_t prg_start = nes::Cartridge::HEADER_SIZE + nes::Cartridge::TRAINER_SIZE;
    data[prg_start] = 0x77;
    const TempRomFile rom(data);

    const nes::Cartridge cart(rom.path());

    CHECK(cart.GetPrgRom().size() == nes::Cartridge::PRG_BLOCK_SIZE);
    CHECK(cart.GetPrgRom()[0] == 0x77);
}

TEST_CASE("Cartridge reports battery-backed RAM when the battery flag is set") {
    auto data = MakeHeader(1, 1, nes::Cartridge::BATTERY_MASK, 0);
    data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE + nes::Cartridge::CHR_BLOCK_SIZE, 0);
    const TempRomFile rom(data);

    const nes::Cartridge cart(rom.path());

    CHECK(cart.HasBatteryBackedRam());
}

TEST_CASE("Cartridge constructor throws when the mapper id is unsupported") {
    // flags_6 top nibble = 0xA, flags_7 top nibble = 0x5 -> mapper 0x5A (90), which has no Mapper implementation
    auto data = MakeHeader(1, 1, 0xA0, 0x50);
    data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE + nes::Cartridge::CHR_BLOCK_SIZE, 0);
    const TempRomFile rom(data);

    CHECK_THROWS_AS(nes::Cartridge(rom.path()), std::runtime_error);
}

TEST_CASE("Cartridge constructs a mapper wired to the parsed PRG-ROM and CHR-ROM data") {
    auto data = MakeHeader(1, 1, 0, 0); // mapper id 0 (NROM)
    data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE + nes::Cartridge::CHR_BLOCK_SIZE, 0);
    data[16] = 0x11;                                                     // first PRG-ROM byte
    data[16 + nes::Cartridge::PRG_BLOCK_SIZE] = 0x22;                    // first CHR-ROM byte
    const TempRomFile rom(data);

    const nes::Cartridge cart(rom.path());

    CHECK(cart.GetMapperId() == 0);
    CHECK(cart.GetMapper().ReadPrg(0x8000) == 0x11);
    CHECK(cart.GetMapper().ReadChr(0x0000) == 0x22);
}

TEST_CASE("Cartridge parses vertical mirroring from flags_6") {
    auto data = MakeHeader(1, 1, nes::Cartridge::MIRROR_MASK, 0);
    data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE + nes::Cartridge::CHR_BLOCK_SIZE, 0);
    const TempRomFile rom(data);

    const nes::Cartridge cart(rom.path());

    CHECK(cart.GetMirroring() == nes::Cartridge::Mirroring::Vertical);
}

TEST_CASE("Cartridge constructor throws when the file is too small to be an iNES file") {
    const TempRomFile rom(std::vector<std::uint8_t>(10, 0));
    CHECK_THROWS_AS(nes::Cartridge(rom.path()), std::runtime_error);
}

TEST_CASE("Cartridge constructor throws when the magic number is missing") {
    auto data = MakeHeader(1, 1, 0, 0);
    data[0] = 0x00;
    data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE + nes::Cartridge::CHR_BLOCK_SIZE, 0);
    const TempRomFile rom(data);

    CHECK_THROWS_AS(nes::Cartridge(rom.path()), std::runtime_error);
}

TEST_CASE("Cartridge constructor throws when the file is truncated before the declared PRG/CHR data ends") {
    // Header claims 1 PRG bank and 1 CHR bank but the file only contains the header
    const TempRomFile rom(MakeHeader(1, 1, 0, 0));
    CHECK_THROWS_AS(nes::Cartridge(rom.path()), std::runtime_error);
}

TEST_CASE("Cartridge constructor throws when the file does not exist") {
    CHECK_THROWS_AS(nes::Cartridge("/no/such/file/definitely_missing.nes"), std::runtime_error);
}