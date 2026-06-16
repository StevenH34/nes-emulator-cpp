//
// Created by Steven Hedges on 6/3/26.
//

#ifndef NES_EMULATOR_CPP_CPU_H
#define NES_EMULATOR_CPP_CPU_H

#include <cstdint>
#include <unordered_map>

#include "Bus.h"

namespace nes {

class Bus;

class Cpu {
public:
    explicit Cpu(Bus& bus);

    static void Debugging();
    static std::string StatusString();

    static std::uint8_t Read(const std::uint16_t address);
    static void Write(const std::uint16_t address, const std::uint8_t value);
    static int Step();
    static std::uint8_t FetchByte();
    static std::uint8_t ReadByte(const std::uint16_t address);

    // LDA Instructions
    static void Lda(const std::uint8_t value);
    static void LdaImmediate();

    // Register Increment Instructions
    static void Inx();

    // Flag instructions
    static void SetFlag(const std::uint8_t mask, const bool is_on);
    static void SetZFlag(const std::uint8_t register_value);
    static void SetNFlag(const std::uint8_t register_value);

private:
    Bus& bus_;

    // CPU Registers
    inline static std::uint8_t accumulator_ = 0;        // Accumulator
    inline static std::uint8_t x_register_ = 0;         // X Register
    inline static std::uint8_t y_register_ = 0;         // Y Register
    inline static std::uint8_t stack_pointer_ = 0xfd;   // Stack Pointer
    inline static std::uint8_t status_register_ = 0x24; // Status Register (flags)
    inline static std::uint16_t program_counter_ = 0;   // Program Counter

    // Flag Masks (N V U B D I Z C)
    // Flags are bits in the Status register.
    // The flags need to flip on or off.
    // Flags are used as a mask to cover up bits we don't want.
    static constexpr std::uint8_t FLAG_C = 0x01; // Carry
    static constexpr std::uint8_t FLAG_Z = 0x02; // Zero
    static constexpr std::uint8_t FLAG_I = 0x04; // Interrupt Disable
    static constexpr std::uint8_t FLAG_D = 0x08; // Decimal
    static constexpr std::uint8_t FLAG_B = 0x10; // Break
    static constexpr std::uint8_t FLAG_U = 0x20; // Unused (always 1)
    static constexpr std::uint8_t FLAG_V = 0x40; // Overflow
    static constexpr std::uint8_t FLAG_N = 0x80; // Negative

    // CPU OpCodes
    struct Opcodes {
        // LDA
        static constexpr std::uint8_t LDA_IMMEDIATE = 0xa9;
        // INX
        static constexpr std::uint8_t INX = 0xe8;
        // Cycles
        inline static const std::unordered_map<std::uint8_t, int> CYCLES = {
            {LDA_IMMEDIATE, 2},
            {INX, 2}
        };

        /*
         * I could also use an array. I'm not sure if it's better.
         * What do C emulators do?
         *
         * inline static constexpr std::array<int, 256> CYCLES = [] {
         *  std::array<int, 256> cycles{};
         *  cycles[LDA_IMMEDIATE] = 2;
         *  cycles[INX] = 2;
         *  return cycles
         * }
         *
         * Then used with:
         * auto opcode = Cpu::Opcodes::INX;
         * auto cycles = Cpu::Opcodes[opcode];
         *
         */
    };
};

} // nes

#endif //NES_EMULATOR_CPP_CPU_H
