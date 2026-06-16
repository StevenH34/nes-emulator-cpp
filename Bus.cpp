//
// Created by Steven Hedges on 6/3/26.
//

#include "Bus.h"

#include <iostream>
#include <ostream>

namespace nes {

Bus::Bus() = default;
Bus::~Bus() = default;

uint8_t Bus::Read(const uint16_t address) {
    std::cout << "Bus read address: "<< address << '\n';
    return 0;
}

void Bus::Write(uint16_t address, uint8_t value) {
    std::cout << "Bus write" << '\n';
}

std::uint8_t Bus::ReadCpu(const std::uint16_t address) {
    if (address >= Bus::RAM_START && address <= Bus::RAM_MIRROR_END) {
        return ReadRam(address);
    }

    return 0;
}

void Bus::WriteCpu(const std::uint16_t address, const std::uint8_t value) {
    if (address >= Bus::RAM_START && address <= Bus::RAM_MIRROR_END) {
        WriteRam(address, value);
    }
}

std::uint8_t Bus::ReadRam(const std::uint16_t address) {
    const auto mirrored_address = (address & Bus::RAM_MASK);
    return Bus::ram_[mirrored_address];
}

void Bus::WriteRam(const std::uint16_t address, const std::uint8_t value) {
    const auto mirrored_address = (address & Bus::RAM_MASK);
    Bus::ram_[mirrored_address] = value;
}

} // nes