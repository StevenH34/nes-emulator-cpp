//
// Created by Steven Hedges on 6/3/26.
//

#include "Cpu.h"

#include <print>

namespace nes {

Cpu::Cpu(Bus& bus) : bus_(bus) {}

// Not a good long-term place to put this
void Cpu::PrintDebugging() {
    std::println("A={:02X}, X={:02X}, Y={:02X}, SP={:02X}, PC={:04X} [{}]",
    accumulator_, x_register_, y_register_,
        stack_pointer_, program_counter_, StatusString());
}

// Should be static or in an anonymous namespace
inline constexpr bool bit(const std::uint8_t value, const int n)
{
    return (value >> n) & 1;
}

std::string Cpu::StatusString() const {
    auto s = status_register_;
    std::string output;
    output.reserve(8);

    output += (bit(s, 7) ? 'N' : 'n');
    output += (bit(s, 6) ? 'V' : 'v');
    output += (bit(s, 5) ? 'U' : 'u');
    output += (bit(s, 4) ? 'B' : 'b');
    output += (bit(s, 3) ? 'D' : 'd');
    output += (bit(s, 2) ? 'I' : 'i');
    output += (bit(s, 1) ? 'Z' : 'z');
    output += (bit(s, 0) ? 'C' : 'c');

    return output;
}

int Cpu::Step() {
    // Get the opcode
    const auto opcode = FetchByte();
    // Execute opcode
    switch (opcode) {
        case Opcodes::LDA_IMMEDIATE:
            LdaImmediate();
            break;
        case Opcodes::STA_ZERO_PAGE:
            StaZeroPage();
            break;
        case Opcodes::STA_ABSOLUTE:
            StaAbsolute();
            break;
        case Opcodes::INX:
            Inx();
            break;
        default:
            throw std::runtime_error("Invalid opcode");
    }
    // Return number of cycles
    return Opcodes::CYCLES[opcode];
}

// Reads byte at the current Program Counter, then increments Program Counter
std::uint8_t Cpu::FetchByte() {
    const auto value = ReadByte(program_counter_);
    program_counter_ += 1;
    return value;
}

std::uint8_t Cpu::ReadByte(const std::uint16_t address) const {
    return bus_.ReadCpu(address);
}

void Cpu::WriteByte(const std::uint16_t address, const std::uint8_t value) {
    bus_.WriteCpu(address, value);
}

/// Addressing Modes
// Read a byte and convert it to a 16-bit address
std::uint16_t Cpu::AddressZeroPage() {
    return FetchByte();
}

// Read two bytes then combine them
std::uint16_t Cpu::AddressAbsolute() {
    const auto low_byte = FetchByte();
    const auto high_byte = FetchByte();
    // Move high_byte because of little endian
    return static_cast<uint16_t>((high_byte << 8) | low_byte);
}

/// STA Instructions
// STA does not affect any flags
void Cpu::StaZeroPage() {
    const auto address = AddressZeroPage();
    WriteByte(address, accumulator_);
}

void Cpu::StaAbsolute() {
    const auto address = AddressAbsolute();
    WriteByte(address, accumulator_);
}

/// LDA Instructions
void Cpu::Lda(const std::uint8_t value) {
    accumulator_ = value;
    SetZFlag(accumulator_);
    SetNFlag(accumulator_);
}

void Cpu::LdaImmediate() {
    const auto value = FetchByte();
    Lda(value);
}

/// Increment Register
void Cpu::Inx() {
    x_register_ += 1;
    SetZFlag(x_register_);
    SetNFlag(x_register_);
}

/// Flag instructions
void Cpu::SetFlag(const StatusFlag flag, const bool is_on) {
    const auto mask = static_cast<std::uint8_t>(flag);
    if (is_on) {
        // Use OR to turn the bit on.
        status_register_ |= mask;
    } else {
        // Use AND with inverted mask to turn the bit off.
        status_register_ &= static_cast<std::uint8_t>(~mask);
    }
}

// Turns the Zero Flag on when the Most Significant Bit (bit 7) is 1.
// This means the number is negative in two's complement.
void Cpu::SetZFlag(const std::uint8_t register_value) {
    SetFlag(StatusFlag::Z, register_value == 0);
}

void Cpu::SetNFlag(const std::uint8_t register_value) {
    SetFlag(StatusFlag::N, ((register_value >> 7) & 1) == 1); // Most Significant Bit
};

} // nes