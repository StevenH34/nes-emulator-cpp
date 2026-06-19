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
    // A full 16-bit address
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



    // Register Increment Instructions
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
    void SetZFlag(std::uint8_t register_value);
    void SetNFlag(std::uint8_t register_value);

private:
    Bus& bus_;

    // CPU Registers
    std::uint8_t accumulator_ = 0;        // Accumulator
    std::uint8_t x_register_ = 0;         // X Register
    std::uint8_t y_register_ = 0;         // Y Register
    std::uint8_t stack_pointer_ = 0xfd;   // Stack Pointer
    std::uint8_t status_register_ = 0x24; // Status Register (flags)
    std::uint16_t program_counter_ = 0;   // Program Counter

    // There's probably a better way to do this
    // CPU OpCodes
    struct Opcodes {
        // LDA
        static constexpr std::uint8_t LDA_IMMEDIATE = 0xa9;
        // STA
        static constexpr std::uint8_t STA_ZERO_PAGE = 0x85;
        static constexpr std::uint8_t STA_ABSOLUTE = 0x8d;
        // Register Increments
        static constexpr std::uint8_t INX = 0xe8;


        inline static constexpr std::array<int, 256> CYCLES = [] {
            std::array<int, 256> cycles{};
            cycles[LDA_IMMEDIATE] = 2;
            cycles[STA_ZERO_PAGE] = 3;
            cycles[STA_ABSOLUTE] = 4;
            cycles[INX] = 2;
            return cycles;
        }();
    };
};

} // nes

#endif //NES_EMULATOR_CPP_CPU_H
