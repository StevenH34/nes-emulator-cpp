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

private:
    Cartridge& cartridge_;
    std::function<void()> nmi_callback_{nullptr};
    int cycle_{0};
    int scanline_{0};
    bool frame_complete_{false};
    std::vector<std::uint8_t> frame_buffer_ = std::vector<std::uint8_t>(WIDTH * HEIGHT * 4, 0);
    std::uint8_t vram_buffer_{0};
    std::vector<std::uint8_t> nametable_ram = std::vector<std::uint8_t>(PpuAddresses::NAMETABLE_RAM_SIZE, 0);
    std::vector<std::uint8_t> palette_ram = std::vector<std::uint8_t>(PpuAddresses::PALETTE_RAM_SIZE, 0);
    std::vector<std::uint8_t> oam = std::vector<std::uint8_t>(PpuAddresses::OAM_SIZE, 0);
    std::uint8_t ctrl_register_{0};
    std::uint8_t mask_register_{0};
    std::uint8_t status_register_{0};
    std::uint8_t oam_addr_register_{0};
    std::uint16_t v_register_{0};
    std::uint16_t t_register_{0};
    std::uint8_t fin_x_register_{0};
    bool w_register_{false};
};

} // namespace nes

#endif //NES_EMULATOR_CPP_PPU_H
