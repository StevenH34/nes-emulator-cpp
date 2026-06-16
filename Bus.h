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
    ~Bus();
    static uint8_t Read(const std::uint16_t address) ;
    static void Write(std::uint16_t address, std::uint8_t value);
    [[nodiscard]] static std::uint8_t ReadCpu(const std::uint16_t address);
    static void WriteCpu(const std::uint16_t address, const std::uint8_t value);
    [[nodiscard]] static std::uint8_t ReadRam(const std::uint16_t address);
    static void WriteRam(const std::uint16_t address, const std::uint8_t value);
    

    // Maybe I should move these into a Struct or something else
    static constexpr std::uint16_t RAM_START = 0x0000;
private:
    // std::vector<std::uint8_t> ram_;
    inline static std::array<std::uint8_t, 2048> ram_{};

    // Addresses
    static constexpr std::uint16_t RAM_SIZE = 2048; // 2 KB

    static constexpr std::uint16_t RAM_END = 0x07FF;
    static constexpr std::uint16_t RAM_MIRROR_END = 0x1FFF;
    static constexpr std::uint16_t RAM_MASK = 0x07FF;
};

} // nes

#endif //NES_EMULATOR_CPP_BUS_H
