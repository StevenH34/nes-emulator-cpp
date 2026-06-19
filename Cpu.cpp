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

// Zero Page + X offset
std::uint16_t Cpu::AddressZeroPageX() {
    const auto base = FetchByte();
    return static_cast<std::uint8_t>(base + x_register_);
}

// Zero Page + Y offset
std::uint16_t Cpu::AddressZeroPageY() {
    const auto base = FetchByte();
    return static_cast<std::uint8_t>(base + y_register_);
}

// Read two bytes then combine them
std::uint16_t Cpu::AddressAbsolute() {
    const auto low_byte = FetchByte();
    const auto high_byte = FetchByte();
    // Move high_byte because of little endian
    return static_cast<uint16_t>(high_byte << 8 | low_byte);
}

// Absolute + X offset
std::uint16_t Cpu::AddressAbsoluteX() {
    return static_cast<std::uint16_t>(AddressAbsolute() + x_register_);
}

// Absolute + Y offset
std::uint16_t Cpu::AddressAbsoluteY() {
    return static_cast<std::uint16_t>(AddressAbsolute() + y_register_);
}

// Relative addressing is used for branch instructions.
// It provides a signed offset (-128 to 127) to the current Program Counter.
std::uint16_t Cpu::AddressRelative() {
    const auto offest  = static_cast<std::int8_t>(FetchByte());
    return static_cast<std::uint16_t>(program_counter_ + offest);
}

// Indirect: JMP ($nnnn)
// Reads a 16-bit address from the pointer location, then jumps to that address.
std::uint16_t Cpu::AddressIndirect() {
    const auto pointer_address = AddressAbsolute();
    const auto low_byte = ReadByte(pointer_address);

    // Emulate the 6502-page-boundary bug
    const std::uint16_t high_byte_address = (pointer_address & 0xFF) == 0xFF
        ? pointer_address & 0xFF00 // Wrap around to the start of the page $xx00
        : pointer_address + 1;     // Normal case

    const auto high_byte = ReadByte(high_byte_address);
    return static_cast<std::uint16_t>(static_cast<uint16_t>(high_byte) << 8 | static_cast<uint16_t>(low_byte));
}

// Indexed Indirect: LDA ($nn,X)
// Adds X to Zero-Page address, then reads a 16-bit pointer address
std::uint16_t Cpu::AddressIndirectX() {
    const auto base = FetchByte();
    const auto pointer = static_cast<uint8_t>(base + x_register_);
    const auto low_byte = ReadByte(pointer);
    // The high-byte pointer also wraps within the zero page
    const auto high_byte = ReadByte(static_cast<std::uint8_t>(pointer + 1));
    return static_cast<std::uint16_t>(high_byte << 8) | static_cast<uint16_t>(low_byte);
}

std::uint16_t Cpu::AddressIndirectY() {
    const auto base = FetchByte();
    const auto low_byte = ReadByte(base);
    // The high-byte pointer wraps within the zero page
    const auto high_byte = ReadByte(static_cast<std::uint8_t>(base + 1));
    const auto addr = static_cast<std::uint16_t>(high_byte << 8) | static_cast<uint16_t>(low_byte);
    return addr + static_cast<uint16_t>(y_register_);
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

void Cpu::LdaZeroPage() {
    const auto address = AddressZeroPage();
    const auto value = ReadByte(address);
    Lda(value);
}

void Cpu::LdaZeroPageX() {
    const auto address = AddressZeroPageX();
    const auto value = ReadByte(address);
    Lda(value);
}

void Cpu::LdaAbsolute() {
    const auto address = AddressAbsolute();
    const auto value = ReadByte(address);
    Lda(value);
}

void Cpu::LdaAbsoluteX() {
    const auto address = AddressAbsoluteX();
    const auto value = ReadByte(address);
    Lda(value);
}

void Cpu::LdaAbsoluteY() {
    const auto address = AddressAbsoluteY();
    const auto value = ReadByte(address);
    Lda(value);
}

void Cpu::LdaIndirectX() {
    const auto address = AddressIndirectX();
    const auto value = ReadByte(address);
    Lda(value);
}

void Cpu::LdaIndirectY() {
    const auto address = AddressIndirectY();
    const auto value = ReadByte(address);
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