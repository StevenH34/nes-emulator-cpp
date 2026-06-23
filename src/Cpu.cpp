//
// Created by Steven Hedges on 6/3/26.
//

#include "Cpu.h"

#include <print>

namespace nes {

Cpu::Cpu(Bus& bus) : bus_(bus) {}

void Cpu::PrintDebugging() {
    std::println("A={:02X}, X={:02X}, Y={:02X}, SP={:02X}, PC={:04X} [{}]",
    accumulator_, x_register_, y_register_,
        stack_pointer_, program_counter_, StatusString());
}

std::string Cpu::StatusString() const {
    auto s = status_register_;
    std::string output;
    output.reserve(8);

    output += s >> 7 & 1 ? 'N' : 'n';
    output += s >> 6 & 1 ? 'V' : 'v';
    output += s >> 5 & 1 ? 'U' : 'u';
    output += s >> 4 & 1 ? 'B' : 'b';
    output += s >> 3 & 1 ? 'D' : 'd';
    output += s >> 2 & 1 ? 'I' : 'i';
    output += s >> 1 & 1 ? 'Z' : 'z';
    output += s >> 0 & 1 ? 'C' : 'c';

    return output;
}

int Cpu::Step() {
    // Get the opcode
    const auto opcode = FetchByte();
    // Execute opcode
    switch (opcode) {
        // LDA
        case Opcodes::LDA_IMMEDIATE:
            LdaImmediate();
            break;
        case Opcodes::LDA_ZERO_PAGE:
            LdaZeroPage();
            break;
        case Opcodes::LDA_ZERO_PAGE_X:
            LdaZeroPageX();
            break;
        case Opcodes::LDA_ABSOLUTE:
            LdaAbsolute();
            break;
        case Opcodes::LDA_ABSOLUTE_X:
            LdaAbsoluteX();
            break;
        case Opcodes::LDA_ABSOLUTE_Y:
            LdaAbsoluteY();
            break;
        case Opcodes::LDA_INDIRECT_X:
            LdaIndirectX();
            break;
        case Opcodes::LDA_INDIRECT_Y:
            LdaIndirectY();
            break;
        // LDX
        case Opcodes::LDX_IMMEDIATE:
            LdxImmediate();
            break;
        case Opcodes::LDX_ZERO_PAGE:
            LdxZeroPage();
            break;
        case Opcodes::LDX_ZERO_PAGE_Y:
            LdxZeroPageY();
            break;
        case Opcodes::LDX_ABSOLUTE:
            LdxAbsolute();
            break;
        case Opcodes::LDX_ABSOLUTE_Y:
            LdxAbsoluteY();
            break;
        // LDY
        case Opcodes::LDY_IMMEDIATE:
            LdyImmediate();
            break;
        case Opcodes::LDY_ZERO_PAGE:
            LdyZeroPage();
            break;
        case Opcodes::LDY_ZERO_PAGE_X:
            LdyZeroPageX();
            break;
        case Opcodes::LDY_ABSOLUTE:
            LdyAbsolute();
            break;
        case Opcodes::LDY_ABSOLUTE_X:
            LdyAbsoluteX();
            break;
        // STA
        case Opcodes::STA_ZERO_PAGE:
            StaZeroPage();
            break;
        case Opcodes::STA_ZERO_PAGE_X:
            StaZeroPageX();
            break;
        case Opcodes::STA_ABSOLUTE:
            StaAbsolute();
            break;
        case Opcodes::STA_ABSOLUTE_X:
            StaAbsoluteX();
            break;
        case Opcodes::STA_ABSOLUTE_Y:
            StaAbsoluteY();
            break;
        case Opcodes::STA_INDIRECT_X:
            StaIndirectX();
            break;
        case Opcodes::STA_INDIRECT_Y:
            StaIndirectY();
            break;
        // STX
        case Opcodes::STX_ZERO_PAGE:
            StxZeroPage();
            break;
        case Opcodes::STX_ZERO_PAGE_Y:
            StxZeroPageY();
            break;
        case Opcodes::STX_ABSOLUTE:
            StxAbsolute();
            break;
        // STY
        case Opcodes::STY_ZERO_PAGE:
            StyZeroPage();
            break;
        case Opcodes::STY_ZERO_PAGE_X:
            StyZeroPageX();
            break;
        case Opcodes::STY_ABSOLUTE:
            StyAbsolute();
            break;
        // Register Increments
        case Opcodes::INX:
            Inx();
            break;
        default: //TODO: Figure out a good way to deal with unsupported Opcodes
            throw std::runtime_error("Invalid opcode");
    }

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
    return high_byte << 8 | low_byte;
}

// Absolute + X offset
std::uint16_t Cpu::AddressAbsoluteX() {
    return AddressAbsolute() + x_register_;
}

// Absolute + Y offset
std::uint16_t Cpu::AddressAbsoluteY() {
    return AddressAbsolute() + y_register_;
}

// Relative addressing is used for branch instructions.
// It provides a signed offset (-128 to 127) to the current Program Counter.
std::uint16_t Cpu::AddressRelative() {
    const auto offest  = static_cast<std::int8_t>(FetchByte());
    return program_counter_ + offest;
}

// Indirect: JMP ($nnnn)
// Reads a 16-bit address from the pointer location, then jumps to that address.
std::uint16_t Cpu::AddressIndirect() {
    const std::uint16_t pointer_address = AddressAbsolute();
    const std::uint8_t low_byte = ReadByte(pointer_address);

    // Emulate the 6502-page-boundary bug
    const std::uint16_t high_byte_address = (pointer_address & 0xFF) == 0xFF
        ? pointer_address & 0xFF00 // Wrap around to the start of the page $xx00
        : pointer_address + 1;     // Normal case

    const std::uint8_t high_byte = ReadByte(high_byte_address);
    return static_cast<uint16_t>(high_byte) << 8 | static_cast<uint16_t>(low_byte);
}

// Indexed Indirect: LDA ($nn,X)
// Adds X to Zero-Page address, then reads a 16-bit pointer address
std::uint16_t Cpu::AddressIndirectX() {
    const std::uint8_t base = FetchByte();
    const auto pointer = static_cast<uint8_t>(base + x_register_);
    const std::uint8_t low_byte = ReadByte(pointer);
    // The high-byte pointer also wraps within the zero page
    const std::uint8_t high_byte = ReadByte(static_cast<std::uint8_t>(pointer + 1));
    return static_cast<std::uint16_t>(high_byte << 8) | static_cast<uint16_t>(low_byte);
}

std::uint16_t Cpu::AddressIndirectY() {
    const std::uint8_t base = FetchByte();
    const std::uint8_t low_byte = ReadByte(base);
    // The high-byte pointer wraps within the zero page
    const std::uint8_t high_byte = ReadByte(static_cast<std::uint8_t>(base + 1));
    const std::uint16_t address = static_cast<std::uint16_t>(high_byte << 8) | static_cast<std::uint16_t>(low_byte);
    return address + y_register_;
}

/// STA Instructions
// STA does not affect any flags
void Cpu::StaZeroPage() {
    const auto address = AddressZeroPage();
    WriteByte(address, accumulator_);
}

void Cpu::StaZeroPageX() {
    const auto address = AddressZeroPageX();
    WriteByte(address, accumulator_);
}

void Cpu::StaAbsolute() {
    const auto address = AddressAbsolute();
    WriteByte(address, accumulator_);
}

void Cpu::StaAbsoluteX() {
    const auto address = AddressAbsoluteX();
    WriteByte(address, accumulator_);
}

void Cpu::StaAbsoluteY() {
    const auto address = AddressAbsoluteY();
    WriteByte(address, accumulator_);
}

void Cpu::StaIndirectX() {
    const auto address = AddressIndirectX();
    WriteByte(address, accumulator_);
}

void Cpu::StaIndirectY() {
    const auto address = AddressIndirectY();
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

/// LDX Instructions
void Cpu::Ldx(const std::uint8_t value) {
    x_register_ = value;
    SetZFlag(x_register_);
    SetNFlag(x_register_);
}

void Cpu::LdxImmediate() {
    const auto value = FetchByte();
    Ldx(value);
}

void Cpu::LdxZeroPage() {
    const auto address = AddressZeroPage();
    const auto value = ReadByte(address);
    Ldx(value);
}

void Cpu::LdxZeroPageY() {
    const auto address = AddressZeroPageY();
    const auto value = ReadByte(address);
    Ldx(value);
}

void Cpu::LdxAbsolute() {
    const auto address = AddressAbsolute();
    const auto value = ReadByte(address);
    Ldx(value);
}

void Cpu::LdxAbsoluteY() {
    const auto address = AddressAbsoluteY();
    const auto value = ReadByte(address);
    Ldx(value);
}

/// LDY Instructions
void Cpu::Ldy(const std::uint8_t value) {
    y_register_ = value;
    SetZFlag(y_register_);
    SetNFlag(y_register_);
}

void Cpu::LdyImmediate() {
    const auto value = FetchByte();
    Ldy(value);
}
void Cpu::LdyZeroPage() {
    const auto address = AddressZeroPage();
    const auto value = ReadByte(address);
    Ldy(value);
}

void Cpu::LdyZeroPageX() {
    const auto address = AddressZeroPageX();
    const auto value = ReadByte(address);
    Ldy(value);
}
void Cpu::LdyAbsolute() {
    const auto address = AddressAbsolute();
    const auto value = ReadByte(address);
    Ldy(value);
}

void Cpu::LdyAbsoluteX() {
    const auto address = AddressAbsoluteX();
    const auto value = ReadByte(address);
    Ldy(value);
}

/// STX Instructions
void Cpu::StxZeroPage() {
    const auto address = AddressZeroPage();
    WriteByte(address, x_register_);
}

void Cpu::StxZeroPageY() {
    const auto address = AddressZeroPageY();
    WriteByte(address, x_register_);
}

void Cpu::StxAbsolute() {
    const auto address = AddressAbsolute();
    WriteByte(address, x_register_);
}

/// STY Instructions
void Cpu::StyZeroPage() {
    const auto address = AddressZeroPage();
    WriteByte(address, y_register_);
}

void Cpu::StyZeroPageX() {
    const auto address = AddressZeroPageX();
    WriteByte(address, y_register_);
}

void Cpu::StyAbsolute() {
    const auto address = AddressAbsolute();
    WriteByte(address, y_register_);
}

/// Increment Register
void Cpu::Inx() {
    x_register_ += 1;
    SetZFlag(x_register_);
    SetNFlag(x_register_);
}

/// Flag Instructions
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

bool Cpu::IsFlagSet(const std::uint8_t mask) const {
    return (status_register_ & mask) != 0;
}

// Turns the Zero Flag on when the Most Significant Bit (bit 7) is 1.
// This means the number is negative in two's complement.
void Cpu::SetZFlag(const std::uint8_t register_value) {
    SetFlag(StatusFlag::Z, register_value == 0);
}

void Cpu::SetNFlag(const std::uint8_t register_value) {
    SetFlag(StatusFlag::N, (register_value >> 7 & 1) == 1); // Most Significant Bit
};

void Cpu::SetCFlag(const bool is_on) {
    SetFlag(StatusFlag::C, is_on);
}

void Cpu::SetVFlag(const bool is_on) {
    SetFlag(StatusFlag::V, is_on);
}

/// Branch Instructions
void Cpu::BranchIf(const bool condition) {
    const auto target = AddressRelative();
    if (condition) program_counter_ = target;
}

void Cpu::Beq() {
    BranchIf(IsFlagSet(static_cast<std::uint8_t>(StatusFlag::Z)));
}

void Cpu::Bne() {
    BranchIf(!IsFlagSet(static_cast<std::uint8_t>(StatusFlag::Z)));
}

void Cpu::Bcs() {
    BranchIf(IsFlagSet(static_cast<std::uint8_t>(StatusFlag::C)));
}

void Cpu::Bcc() {
    BranchIf(!IsFlagSet(static_cast<std::uint8_t>(StatusFlag::C)));
}

void Cpu::Bmi() {
    BranchIf(IsFlagSet(static_cast<std::uint8_t>(StatusFlag::N)));
}

void Cpu::Bpl() {
    BranchIf(!IsFlagSet(static_cast<std::uint8_t>(StatusFlag::N)));
}

void Cpu::Bvs() {
    BranchIf(IsFlagSet(static_cast<std::uint8_t>(StatusFlag::V)));
}

void Cpu::Bvc() {
    BranchIf(!IsFlagSet(static_cast<std::uint8_t>(StatusFlag::V)));
}

/// Jump Instructions
void Cpu::JmpAbsolute() {
    // Reads 16-bit address and sets PC to it
    program_counter_ = AddressAbsolute();
}

void Cpu::JmpIndirect() {
    // Reads 16-bit address where destination is stored and sets PC to it
    program_counter_ = AddressIndirect();
}

/// Stack

void Cpu::StackPushByte(const std::uint8_t value) {
    // Write current value at stack address then decrement stack pointer
    WriteByte(STACK_BASE_ | static_cast<std::uint16_t>(stack_pointer_), value);
    stack_pointer_ -= 1;
}

std::uint8_t Cpu::StackPopByte() {
    // Decrements stack pointer then reads address
    stack_pointer_ += 1;
    return ReadByte(STACK_BASE_ | static_cast<std::uint16_t>(stack_pointer_));
}

void Cpu::StackPushWord(const std::uint16_t value) {
    StackPushByte(static_cast<std::uint8_t>(value >> 8)); // High byte
    StackPushByte(static_cast<std::uint8_t>(value & 0xFF)); // Low byte
}

std::uint16_t Cpu::StackPopWord() {
    const std::uint8_t low_byte = StackPopByte();
    const std::uint8_t high_byte = StackPopByte();
    return static_cast<std::uint16_t>(high_byte << 8) | static_cast<std::uint16_t>(low_byte);
}

} // nes