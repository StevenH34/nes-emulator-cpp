//
// Created by Steven Hedges on 6/3/26.
//

#ifndef NES_EMULATOR_CPP_CPU_H
#define NES_EMULATOR_CPP_CPU_H

#include <cstdint>
#include <array>
#include <string>

#include "Bus.h"

namespace nes {

class Cpu {
public:
    explicit Cpu(Bus& bus);

    void PrintDebugging();
    [[nodiscard]] std::string StatusString() const;
    [[nodiscard]] int Step();
    [[nodiscard]] std::uint8_t FetchByte();
    [[nodiscard]] std::uint8_t ReadByte(std::uint16_t address) const;
    void WriteByte(std::uint16_t address, std::uint8_t value);

    [[nodiscard]] std::uint8_t GetAccumulator() const { return accumulator_; }
    [[nodiscard]] std::uint8_t GetXRegister() const { return x_register_;};
    [[nodiscard]] std::uint8_t GetYRegister() const { return y_register_; }
    [[nodiscard]] std::uint8_t GetStatusRegister() const { return status_register_; }

    // Test helpers for driving register state directly
    void SetXRegister(const std::uint8_t value) { x_register_ = value; }
    void SetYRegister(const std::uint8_t value) { y_register_ = value; }

    /// Addressing Modes
    // Zero Page is an 8-bit address in the first 256 bytes of memory
    std::uint16_t AddressZeroPage();
    std::uint16_t AddressZeroPageX();
    std::uint16_t AddressZeroPageY();
    std::uint16_t AddressAbsolute();
    std::uint16_t AddressAbsoluteX();
    std::uint16_t AddressAbsoluteY();
    std::uint16_t AddressRelative();
    // Indirect addressing is used for JMP (Jump) instructions.
    // It reads a 16-bit address from the instruction, this address is the final destination of the jump.
    std::uint16_t AddressIndirect();
    std::uint16_t AddressIndirectX();
    std::uint16_t AddressIndirectY();

    /// STA Instructions
    void StaZeroPage();
    void StaZeroPageX();
    void StaAbsolute();
    void StaAbsoluteX();
    void StaAbsoluteY();
    void StaIndirectX();
    void StaIndirectY();

    /// LDA Instructions
    void Lda(std::uint8_t value);
    void LdaImmediate();
    void LdaZeroPage();
    void LdaZeroPageX();
    void LdaAbsolute();
    void LdaAbsoluteX();
    void LdaAbsoluteY();
    void LdaIndirectX();
    void LdaIndirectY();

    /// LDX Instructions
    void Ldx(std::uint8_t value);
    void LdxImmediate();
    void LdxZeroPage();
    void LdxZeroPageY();
    void LdxAbsolute();
    void LdxAbsoluteY();

    /// LDY Instructions
    void Ldy(std::uint8_t value);
    void LdyImmediate();
    void LdyZeroPage();
    void LdyZeroPageX();
    void LdyAbsolute();
    void LdyAbsoluteX();

    /// STX Instructions
    void StxZeroPage();
    void StxZeroPageY();
    void StxAbsolute();

    /// STY Instructions
    void StyZeroPage();
    void StyZeroPageX();
    void StyAbsolute();

    /// Register Increments Instruction
    void Inx();

    /// Flag Masks (N V U B D I Z C)
    // Flags are bits in the Status register.
    // The flags need to flip on or off.
    // Flags are used as a mask to cover up bits we don't want.
    enum class StatusFlag : std::uint8_t {
        C = 0x01, // Carry
        Z = 0x02, // Zero
        I = 0x04, // Interrupt Disable
        D = 0x08, // Decimal
        B = 0x10, // Break
        U = 0x20, // Unused (always 1)
        V = 0x40, // Overflow
        N = 0x80  // Negative
    };

    /// Flag instructions
    void SetFlag(StatusFlag flag, bool is_on);
    bool IsFlagSet(std::uint8_t mask) const;
    void SetZFlag(std::uint8_t register_value); // Zero Flag
    void SetNFlag(std::uint8_t register_value); // Negative Flag
    void SetCFlag(bool is_on);  // Carry Flag
    void SetVFlag(bool is_on);  // Overflow Flag

private:
    Bus& bus_;

    /// CPU Registers
    std::uint8_t accumulator_ = 0;        // Accumulator
    std::uint8_t x_register_ = 0;         // X Register
    std::uint8_t y_register_ = 0;         // Y Register
    std::uint8_t stack_pointer_ = 0xfd;   // Stack Pointer
    std::uint8_t status_register_ = 0x24; // Status Register (flags)
    std::uint16_t program_counter_ = 0;   // Program Counter

    /// CPU OpCodes
    struct Opcodes {
        /// LDA Opcodes
        static constexpr std::uint8_t LDA_IMMEDIATE = 0xA9;
        static constexpr std::uint8_t LDA_ZERO_PAGE = 0xA5;
        static constexpr std::uint8_t LDA_ZERO_PAGE_X = 0xB5;
        static constexpr std::uint8_t LDA_ABSOLUTE = 0xAD;
        static constexpr std::uint8_t LDA_ABSOLUTE_X = 0xBD;
        static constexpr std::uint8_t LDA_ABSOLUTE_Y = 0xB9;
        static constexpr std::uint8_t LDA_INDIRECT_X = 0xA1;
        static constexpr std::uint8_t LDA_INDIRECT_Y = 0xB1;

        /// LDX Opcodes
        static constexpr std::uint8_t LDX_IMMEDIATE = 0xA2;
        static constexpr std::uint8_t LDX_ZERO_PAGE = 0xA6;
        static constexpr std::uint8_t LDX_ZERO_PAGE_Y = 0xB6;
        static constexpr std::uint8_t LDX_ABSOLUTE = 0xAE;
        static constexpr std::uint8_t LDX_ABSOLUTE_Y = 0xBE;

        /// LDY Opcodes
        static constexpr std::uint8_t LDY_IMMEDIATE = 0xA0;
        static constexpr std::uint8_t LDY_ZERO_PAGE = 0xA4;
        static constexpr std::uint8_t LDY_ZERO_PAGE_X = 0xB4;
        static constexpr std::uint8_t LDY_ABSOLUTE = 0xAC;
        static constexpr std::uint8_t LDY_ABSOLUTE_X = 0xBC;

        /// STA Opcodes
        static constexpr std::uint8_t STA_ZERO_PAGE = 0x85;
        static constexpr std::uint8_t STA_ZERO_PAGE_X = 0x95;
        static constexpr std::uint8_t STA_ABSOLUTE = 0x8D;
        static constexpr std::uint8_t STA_ABSOLUTE_X = 0x9D;
        static constexpr std::uint8_t STA_ABSOLUTE_Y = 0x99;
        static constexpr std::uint8_t STA_INDIRECT_X = 0x81;
        static constexpr std::uint8_t STA_INDIRECT_Y = 0x91;

        /// STX Opcodes
        static constexpr std::uint8_t STX_ZERO_PAGE = 0x86;
        static constexpr std::uint8_t STX_ZERO_PAGE_Y = 0x96;
        static constexpr std::uint8_t STX_ABSOLUTE = 0x8E;

        /// STY Opcodes
        static constexpr std::uint8_t STY_ZERO_PAGE = 0x84;
        static constexpr std::uint8_t STY_ZERO_PAGE_X = 0x94;
        static constexpr std::uint8_t STY_ABSOLUTE = 0x8C;

        /// Register Increments Opcode
        static constexpr std::uint8_t INX = 0xe8;


        static constexpr std::array<int, 256> CYCLES = [] {
            std::array<int, 256> cycles{};
            // LDA Cycles
            cycles[LDA_IMMEDIATE] = 2;
            cycles[LDA_ZERO_PAGE] = 3;
            cycles[LDA_ZERO_PAGE_X] = 4;
            cycles[LDA_ABSOLUTE] = 4;
            cycles[LDA_ABSOLUTE_X] = 4;
            cycles[LDA_ABSOLUTE_Y] = 4;
            cycles[LDA_INDIRECT_X] = 6;
            cycles[LDA_INDIRECT_Y] = 5;
            // LDX Cycles
            cycles[LDX_IMMEDIATE] = 2;
            cycles[LDX_ZERO_PAGE] = 3;
            cycles[LDX_ZERO_PAGE_Y] = 4;
            cycles[LDX_ABSOLUTE] = 4;
            cycles[LDX_ABSOLUTE_Y] = 4;
            // LDY Cycles
            cycles[LDY_IMMEDIATE] = 2;
            cycles[LDY_ZERO_PAGE] = 3;
            cycles[LDY_ZERO_PAGE_X] = 4;
            cycles[LDY_ABSOLUTE] = 4;
            cycles[LDY_ABSOLUTE_X] = 4;
            // STA Cycles
            cycles[STA_ZERO_PAGE] = 3;
            cycles[STA_ZERO_PAGE_X] = 4;
            cycles[STA_ABSOLUTE] = 4;
            cycles[STA_ABSOLUTE_X] = 5;
            cycles[STA_ABSOLUTE_Y] = 5;
            cycles[STA_INDIRECT_X] = 6;
            cycles[STA_INDIRECT_Y] = 6;
            // STX Cycles
            cycles[STX_ZERO_PAGE] = 3;
            cycles[STX_ZERO_PAGE_Y] = 4;
            cycles[STX_ABSOLUTE] = 4;
            // STY Cycles
            cycles[STY_ZERO_PAGE] = 3;
            cycles[STY_ZERO_PAGE_X] = 4;
            cycles[STY_ABSOLUTE] = 4;
            // Register Increments Cycles
            cycles[INX] = 2;
            return cycles;
        }();
    };
};

} // nes

#endif //NES_EMULATOR_CPP_CPU_H
