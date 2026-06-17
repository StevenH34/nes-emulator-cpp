//
// Created by Steven Hedges on 6/3/26.
//

#include "Bus.h"

#include <iostream>
#include <ostream>

namespace nes {

Bus::Bus() = default;

std::uint8_t Bus::ReadCpu(const std::uint16_t address) const {
    // address 0x000 is always true
    // Not sure if that's a problem
    if (address >= RAM_START && address <= RAM_MIRROR_END) {
        return ReadRam(address);
    }

    return 0;
}

void Bus::WriteCpu(const std::uint16_t address, const std::uint8_t value) {
    if (address >= RAM_START && address <= RAM_MIRROR_END) {
        WriteRam(address, value);
    }
}

std::uint8_t Bus::ReadRam(const std::uint16_t address) const {
    const auto mirrored_address = (address & RAM_MASK);
    return ram_[mirrored_address];
}

void Bus::WriteRam(const std::uint16_t address, const std::uint8_t value) {
    const auto mirrored_address = (address & RAM_MASK);
    ram_[mirrored_address] = value;
}

} // nes