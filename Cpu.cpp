//
// Created by Steven Hedges on 6/3/26.
//

#include "Cpu.h"

#include <print>

namespace nes {

Cpu::Cpu(Bus& bus) : bus_(bus) {}

void Cpu::Debugging() {
    std::println("A={:02X}, X={:02X}, Y={:02X}, SP={:02X}, PC={:04X} [{}]",
    Cpu::accumulator_, Cpu::x_register_, Cpu::y_register_,
         Cpu::stack_pointer_, Cpu::program_counter_, StatusString());
}

constexpr bool bit(const std::uint8_t value, const int n)
{
    return (value >> n) & 1;
}

std::string Cpu::StatusString() {
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

uint8_t Cpu::Read(const uint16_t address) {
    return nes::Bus::Read(address);
};

void Cpu::Write(const uint16_t address, const uint8_t value) {
    nes::Bus::Write(address, value);
}

int Cpu::Step() {
    // Get the opcode
    const auto opcode = Cpu::FetchByte();
    // Execute opcode
    switch (opcode) {
        case Opcodes::LDA_IMMEDIATE:
            Cpu::LdaImmediate();
            break;
        case Opcodes::INX:
            Cpu::Inx();
            break;
        default:
            throw std::runtime_error("Invalid opcode");
    }
    // Return number of cycles
    return Opcodes::CYCLES.at(opcode);
}

// Reads byte at the current Program Counter, then increments Program Counter
std::uint8_t Cpu::FetchByte() {
    const auto value = ReadByte(program_counter_);
    program_counter_ += 1;
    return value;
}

std::uint8_t Cpu::ReadByte(const std::uint16_t address) {
    return Bus::Read(address);
}

void Cpu::Lda(const std::uint8_t value) {
    accumulator_ = value;
    Cpu::SetZFlag(accumulator_);
    Cpu::SetNFlag(accumulator_);
}

void Cpu::LdaImmediate() {
    const auto value = Cpu::FetchByte();
    Cpu::Lda(value);
}

void Cpu::Inx() {
    accumulator_ += 1;
    Cpu::SetZFlag(accumulator_);
    Cpu::SetNFlag(accumulator_);
}

void Cpu::SetFlag(const std::uint8_t mask, const bool is_on) {
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
    const auto is_zero = register_value == 0;
    SetFlag(Cpu::FLAG_Z, is_zero);
}

void Cpu::SetNFlag(const std::uint8_t register_value) {
    const auto seventh_bit = (register_value >> 7) & 1; // Most Significant Bit
    const auto is_on = seventh_bit == 1;
    SetFlag(Cpu::FLAG_N, is_on);
};

} // nes