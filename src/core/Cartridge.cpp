#include "Cartridge.h"

#include <sstream>
#include <filesystem>
#include <stdexcept>
#include <format>
#include <fstream>

namespace nes {

Cartridge::Cartridge(std::string path)
    : path_(std::move(path)) {
    const std::vector<std::uint8_t> data = ReadFileBytes(path_);
    Parse(data);
}

std::vector<std::uint8_t> Cartridge::ReadFileBytes(const std::string& path) {
    std::error_code ec;
    const auto size = std::filesystem::file_size(path, ec);
    if (ec) {
        throw std::runtime_error(std::format("Could not stat file '{}': {}", path, ec.message()));
    }

    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error(std::format("Could not open file: {}", path));
    }

    std::vector<std::uint8_t> data(size);
    if (!file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size))) {
        throw std::runtime_error(std::format("Failed to read file: {}", path));
    }

    return data;
}

void Cartridge::ValidateHeader(const std::span<const std::uint8_t> data) {
    if (data.size() < HEADER_SIZE) {
        std::ostringstream oss;
        oss << "Invalid iNES file: too small (" << data.size()
        << " bytes, minimum " << HEADER_SIZE << ")";
        throw std::runtime_error(oss.str());
    }

    if (!std::equal(NES_HEADER.begin(), NES_HEADER.end(), data.begin(), data.begin() + 4)) {
        throw std::runtime_error("Invalid iNES file: missing NES header");
    }
}

// The upper 4 bits of flags_6 hold the low nibble of the mapper number
// The upper 4 bits of flags_7 hold the high nibble of the mapper number
// OR them into 1 byte (8 bits)
std::uint8_t Cartridge::ParseMapperId(const std::uint8_t flags_6, const std::uint8_t flags_7) {
    const std::uint8_t low = (flags_6 & MAPPER_LOW_MASK) >> 4;
    const std::uint8_t high = flags_7 & MAPPER_HIGH_MASK;
    return high | low;
}

Cartridge::Mirroring Cartridge::ParseMirroring(const std::uint8_t flags_6) {
    // Four-screen mask takes priority
    if (flags_6 & FOUR_SCREEN_MASK) {
        return Mirroring::FourScreen;
    }
    // Check the mirror bit
    // 1 = vertical, 0 = horizontal
    if (flags_6 & MIRROR_MASK) {
        return Mirroring::Vertical;
    }
    return Mirroring::Horizontal;
}

void Cartridge::Parse(std::span<const std::uint8_t> data) {
    // Validate it's an iNES file
    ValidateHeader(data);

    // Read the flag bytes
    const std::uint8_t flags_6 = data[FLAGS_6_BYTE];
    const std::uint8_t flags_7 = data[FLAGS_7_BYTE];
    // Calculate the PRG and CHR rom sizes
    const std::uint32_t prg_rom_size = static_cast<uint32_t>(data[PRG_SIZE_BYTE]) * PRG_BLOCK_SIZE;
    const std::uint32_t chr_rom_size = static_cast<uint32_t>(data[CHR_SIZE_BYTE]) * CHR_BLOCK_SIZE;

    // Parse the mapper ID
    mapper_id_ = ParseMapperId(flags_6, flags_7);
    // Parse the mirroring type
    mirroring_ = ParseMirroring(flags_6);
    // Check for battery-backed RAM
    battery_ = flags_6 & BATTERY_MASK;
    // Check for trainer
    const bool has_trainer = flags_6 & TRAINER_MASK;
    // Calculate the offsets
    // If there’s a 512 byte trainer PRG starts 512 bytes after the header
    // Trainers are not common, but still must be accounted for
    const std::uint32_t prg_rom_start = HEADER_SIZE + (has_trainer ? TRAINER_SIZE : 0);
    const std::uint32_t chr_rom_start = prg_rom_start + prg_rom_size;

    // Make sure the file actually contains the PRG/CHR data the header claims
    if (const std::uint32_t chr_rom_end = chr_rom_start + chr_rom_size; data.size() < chr_rom_end) {
        std::ostringstream oss;
        oss << "Invalid iNES file: expected at least " << chr_rom_end
        << " bytes, got " << data.size();
        throw std::runtime_error(oss.str());
    }

    // Extract PRG-ROM and CHR-ROM data
    prg_rom_.assign(data.begin() + prg_rom_start, data.begin() + prg_rom_start + prg_rom_size);
    // If CHR-ROM is size 0, the cartridge has no CHR-ROM and uses CHR-RAM instead
    // We create 8KB of zeros (empty RAM). The PPU will write graphics at runtime
    if (chr_rom_size > 0) {
        chr_rom_.assign(data.begin() + chr_rom_start, data.begin() + chr_rom_start + chr_rom_size);
    } else {
        chr_rom_.assign(CHR_BLOCK_SIZE, 0u);
    }

    mapper_ = Mapper::Create(mapper_id_, prg_rom_, chr_rom_);
}

}
