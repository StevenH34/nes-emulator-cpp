#include "Mapper.h"
#include "Mapper000.h"

#include <stdexcept>
#include <string>

namespace nes {

std::unique_ptr<Mapper> Mapper::Create(const std::uint8_t id, std::span<const std::uint8_t> prg_rom, std::span<const std::uint8_t> chr_rom) {
    switch (id) {
        case 0:
            return std::make_unique<Mapper000>(prg_rom, chr_rom);
        case 1:
            throw std::runtime_error("Mapper 1 (MMC1) not yet implemented");
        default:
            throw std::runtime_error("Unsupported mapper ID: " + std::to_string(id));
    }
}

} // namespace nes
