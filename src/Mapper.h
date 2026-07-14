#ifndef NES_EMULATOR_CPP_MAPPER_H
#define NES_EMULATOR_CPP_MAPPER_H

#include <cstdint>
#include <memory>
#include <vector>
#include <stdexcept>
#include <string>
#include <span>

namespace nes {

/// Abstract Mapper Class
class Mapper {
public:
    virtual ~Mapper() = default;
    Mapper() = default;
    Mapper(const Mapper&) = delete;
    Mapper& operator=(const Mapper&) = delete;
    Mapper(Mapper&&) = default;
    Mapper& operator=(Mapper&&) = default;

    /// Construct the Mapper subclass for the given iNES mapper ID
    /// @param id The iNES mapper number
    /// @param prg_rom View of the cartridge's PRG-ROM data
    /// @param chr_rom View of the cartridge's CHR-ROM data
    /// @throws std::runtime_error if the mapper ID is unsupported
    static std::unique_ptr<Mapper> Create(std::uint8_t id, std::span<const std::uint8_t> prg_rom, std::span<const std::uint8_t> chr_rom);

    /// Read a byte from the PRG-ROM ($8000-$FFFF)
    /// @param address The address to read from
    /// @return The byte read from the PRG-ROM
    [[nodiscard]] virtual std::uint8_t ReadPrg(std::uint16_t address) const = 0;

    /// CPU writes to mapper ($8000-$FFFF)
    /// @param address The address to write to
    /// @param value The value to write
    virtual void WritePrg(std::uint16_t address, std::uint8_t value) {
        // Default: ignore writes (e.g. Mapper 0 has no registers)
    }

    /// PPU reads CHR-ROM ($0000-$1FFF)
    /// @param address The address to read from
    /// @return The byte read from the CHR-ROM
    [[nodiscard]] virtual std::uint8_t ReadChr(std::uint16_t address) const = 0;

    /// PPU writes to CHR if CHR-RAM is present
    /// @param address The address to write to
    /// @param value The value to write
    virtual void WriteChr(std::uint16_t address, std::uint8_t value) {
        // Default: ignore writes (most mappers don't have CHR-RAM)
    }

    /// CPU reads from WRAM ($6000-$7FFF)
    /// @param address The address to read from
    /// @return The byte read from the WRAM
    [[nodiscard]] virtual std::uint8_t ReadWram(std::uint16_t address) const { return 0; }

    /// CPU writes to WRAM
    /// @param address The address to write to
    /// @param value The value to write
    virtual void WriteWram(std::uint16_t address, std::uint8_t value) {
        // Default: ignore writes (no WRAM is present)
    }
};

} // namespace nes

#endif //NES_EMULATOR_CPP_MAPPER_H
