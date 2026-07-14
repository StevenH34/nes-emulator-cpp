#include "TestRom.h"

#include <atomic>
#include <filesystem>
#include <fstream>

namespace nes_test {

TempRomFile::TempRomFile(const std::vector<std::uint8_t>& data) {
    static std::atomic<int> counter{0};
    path_ = (std::filesystem::temp_directory_path() /
             ("nes_test_rom_" + std::to_string(counter++) + ".nes")).string();
    std::ofstream file(path_, std::ios::binary);
    file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
}

TempRomFile::~TempRomFile() {
    std::error_code ec;
    std::filesystem::remove(path_, ec);
}

std::vector<std::uint8_t> MakeMinimalRom(const std::uint16_t reset_vector) {
    std::vector<std::uint8_t> data{0x4E, 0x45, 0x53, 0x1A, 0x01, 0x01, 0x00, 0x00,
                                   0, 0, 0, 0, 0, 0, 0, 0};
    data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE + nes::Cartridge::CHR_BLOCK_SIZE, 0);

    // Reset vector lives in the last 2 bytes of the 16 KB PRG bank (CPU 0xFFFC/0xFFFD,
    // mirrored down to PRG offset 0x3FFC/0x3FFD by Mapper000's 16 KB mask).
    const auto prg_start = nes::Cartridge::HEADER_SIZE;
    data[prg_start + 0x3FFC] = static_cast<std::uint8_t>(reset_vector & 0xFF);
    data[prg_start + 0x3FFD] = static_cast<std::uint8_t>(reset_vector >> 8);

    return data;
}

nes::Cartridge& GetTestCartridge() {
    static const TempRomFile rom(MakeMinimalRom());
    static nes::Cartridge cartridge(rom.path());
    return cartridge;
}

} // nes_test
