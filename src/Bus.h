//
// Created by Steven Hedges on 6/3/26.
//

#ifndef NES_EMULATOR_CPP_BUS_H
#define NES_EMULATOR_CPP_BUS_H

#include <cstdint>
#include <array>

namespace nes {

class Bus {
public:
    Bus();
    ~Bus() = default;
    [[nodiscard]] std::uint8_t ReadCpu( std::uint16_t address) const;
    void WriteCpu(std::uint16_t address, std::uint8_t value);
    [[nodiscard]] std::uint8_t ReadRam(std::uint16_t address) const;
    void WriteRam(std::uint16_t address, std::uint8_t value);

private:
    // std::vector<std::uint8_t> ram_;
    std::array<std::uint8_t, 2048> ram_{};

    // Addresses
    static constexpr std::uint16_t RAM_SIZE = 2048; // 2 KB
    static constexpr std::uint16_t RAM_START = 0x0000;
    static constexpr std::uint16_t RAM_END = 0x07FF;
    static constexpr std::uint16_t RAM_MIRROR_END = 0x1FFF;
    static constexpr std::uint16_t RAM_MASK = 0x07FF;
};

} // nes

#endif //NES_EMULATOR_CPP_BUS_H
