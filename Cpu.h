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
    [[nodiscard]] std::uint8_t XRegister() const;

    // LDA Instructions
    void Lda(std::uint8_t value);
    void LdaImmediate();

    // Register Increment Instructions
    void Inx();

    // Flag Masks (N V U B D I Z C)
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

    // Flag instructions
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

    // CPU OpCodes
    struct Opcodes {
        // LDA
        static constexpr std::uint8_t LDA_IMMEDIATE = 0xa9;
        // INX
        static constexpr std::uint8_t INX = 0xe8;
        // Cycles
        // inline static const std::unordered_map<std::uint8_t, int> CYCLES = {
        //     {LDA_IMMEDIATE, 2},
        //     {INX, 2}
        // };

         inline static constexpr std::array<int, 256> CYCLES = [] {
          std::array<int, 256> cycles{};
          cycles[LDA_IMMEDIATE] = 2;
          cycles[INX] = 2;
          return cycles;
         }();
         /*
         * Then used with:
         * auto opcode = Cpu::Opcodes::INX;
         * auto cycles = Cpu::Opcodes[opcode];
         *
         */
    };
};

} // nes

#endif //NES_EMULATOR_CPP_CPU_H
