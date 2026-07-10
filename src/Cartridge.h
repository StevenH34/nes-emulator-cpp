#ifndef NES_EMULATOR_CPP_CARTRIDGE_H
#define NES_EMULATOR_CPP_CARTRIDGE_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include <span>

namespace nes {

/**
 * Header structure of an iNES file:
 * Byte 0-3: "NES" + 0x1A (magic number)
 * Byte 4: PRG-ROM blocks (x 16KB)
 * Byte 5: CHR-ROM blocks (x 8KB)
 * Byte 6: Flags 6
 * Byte 7: Flags 7
 * Byte 8-15: Padding (ignored)
 */

class Cartridge {
public:
    explicit Cartridge(std::string path);
    ~Cartridge() = default;

    // iNES Header
    static constexpr std::size_t HEADER_SIZE = 16;
    static constexpr std::array<std::uint8_t, 4> NES_HEADER = {0x4E, 0x45, 0x53, 0x1A}; // "NES\x1A"
    // ROM block sizes
    static constexpr std::uint32_t PRG_BLOCK_SIZE = 0x4000; // 16 KB
    static constexpr std::uint32_t CHR_BLOCK_SIZE = 0x2000;  // 8 KB
    // Trainer
    static constexpr std::uint32_t TRAINER_SIZE = 512;
    // Header byte positions
    static constexpr std::uint32_t PRG_SIZE_BYTE = 4;
    static constexpr std::uint32_t CHR_SIZE_BYTE = 5;
    static constexpr std::uint32_t FLAGS_6_BYTE  = 6;
    static constexpr std::uint32_t FLAGS_7_BYTE  = 7;
    // Flags 6 masks
    static constexpr std::uint8_t MIRROR_MASK      = 0b0000'0001; // 0 = horizontal, 1 = vertical
    static constexpr std::uint8_t BATTERY_MASK     = 0b0000'0010; // 1 = battery-backed RAM
    static constexpr std::uint8_t TRAINER_MASK     = 0b0000'0100; // 1 = 512-byte trainer before PRG
    static constexpr std::uint8_t FOUR_SCREEN_MASK = 0b0000'1000; // 1 = four-screen VRAM
    static constexpr std::uint8_t MAPPER_LOW_MASK  = 0b1111'0000; // Mapper number bits 0-3
    // Flags 7 masks
    static constexpr std::uint8_t MAPPER_HIGH_MASK = 0b1111'0000; // Mapper number bits 4-7

    enum class Mirroring {
        Horizontal,
        Vertical,
        FourScreen
    };

    [[nodiscard]] std::string GetPath() const { return path_; }
    [[nodiscard]] const std::vector<std::uint8_t>& GetPrgRom() const { return prg_rom_; }
    [[nodiscard]] const std::vector<std::uint8_t>& GetChrRom() const { return chr_rom_; }
    [[nodiscard]] Mirroring GetMirroring() const { return mirroring_; }
    [[nodiscard]] std::uint8_t GetMapperId() const { return mapper_id_; }
    [[nodiscard]] bool HasBatteryBackedRam() const { return battery_; }


    static std::vector<std::uint8_t> ReadFileBytes(const std::string& path);
    static void ValidateHeader(std::span<const std::uint8_t> data);
    static std::uint8_t ParseMapperId(std::uint8_t flags_6, std::uint8_t flags_7);
    static Mirroring ParseMirroring(std::uint8_t flags_6);
    void Parse(std::span<const std::uint8_t> data);

private:
    std::string path_;
    std::vector<std::uint8_t> prg_rom_;
    std::vector<std::uint8_t> chr_rom_;
    Mirroring mirroring_{Mirroring::Horizontal};
    bool battery_{false};
    std::uint8_t mapper_id_{0};
};

}

#endif //NES_EMULATOR_CPP_CARTRIDGE_H
