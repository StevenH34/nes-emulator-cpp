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
    Cpu::accumulator_, Cpu::x_register_, Cpu::y_register_,
        Cpu::stack_pointer_, Cpu::program_counter_, StatusString());
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

void Cpu::WriteByte(std::uint16_t address, std::uint8_t value) {
    bus_.WriteCpu(address, value);
}

std::uint8_t Cpu::XRegister() const {
    return x_register_;
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
    x_register_ += 1;
    Cpu::SetZFlag(x_register_);
    Cpu::SetNFlag(x_register_);
}

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