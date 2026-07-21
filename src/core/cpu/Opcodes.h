#ifndef NES_EMULATOR_CPP_OPCODES_H
#define NES_EMULATOR_CPP_OPCODES_H

#include <array>
#include <cstdint>

namespace nes {

/// CPU OpCodes
struct Opcodes {
    /// LDA Opcodes
    static constexpr std::uint8_t LDA_IMMEDIATE   = 0xA9;
    static constexpr std::uint8_t LDA_ZERO_PAGE   = 0xA5;
    static constexpr std::uint8_t LDA_ZERO_PAGE_X = 0xB5;
    static constexpr std::uint8_t LDA_ABSOLUTE    = 0xAD;
    static constexpr std::uint8_t LDA_ABSOLUTE_X  = 0xBD;
    static constexpr std::uint8_t LDA_ABSOLUTE_Y  = 0xB9;
    static constexpr std::uint8_t LDA_INDIRECT_X  = 0xA1;
    static constexpr std::uint8_t LDA_INDIRECT_Y  = 0xB1;

    /// LDX Opcodes
    static constexpr std::uint8_t LDX_IMMEDIATE   = 0xA2;
    static constexpr std::uint8_t LDX_ZERO_PAGE   = 0xA6;
    static constexpr std::uint8_t LDX_ZERO_PAGE_Y = 0xB6;
    static constexpr std::uint8_t LDX_ABSOLUTE    = 0xAE;
    static constexpr std::uint8_t LDX_ABSOLUTE_Y  = 0xBE;

    /// LDY Opcodes
    static constexpr std::uint8_t LDY_IMMEDIATE   = 0xA0;
    static constexpr std::uint8_t LDY_ZERO_PAGE   = 0xA4;
    static constexpr std::uint8_t LDY_ZERO_PAGE_X = 0xB4;
    static constexpr std::uint8_t LDY_ABSOLUTE    = 0xAC;
    static constexpr std::uint8_t LDY_ABSOLUTE_X  = 0xBC;

    /// STA Opcodes
    static constexpr std::uint8_t STA_ZERO_PAGE   = 0x85;
    static constexpr std::uint8_t STA_ZERO_PAGE_X = 0x95;
    static constexpr std::uint8_t STA_ABSOLUTE    = 0x8D;
    static constexpr std::uint8_t STA_ABSOLUTE_X  = 0x9D;
    static constexpr std::uint8_t STA_ABSOLUTE_Y  = 0x99;
    static constexpr std::uint8_t STA_INDIRECT_X  = 0x81;
    static constexpr std::uint8_t STA_INDIRECT_Y  = 0x91;

    /// STX Opcodes
    static constexpr std::uint8_t STX_ZERO_PAGE   = 0x86;
    static constexpr std::uint8_t STX_ZERO_PAGE_Y = 0x96;
    static constexpr std::uint8_t STX_ABSOLUTE    = 0x8E;

    /// STY Opcodes
    static constexpr std::uint8_t STY_ZERO_PAGE   = 0x84;
    static constexpr std::uint8_t STY_ZERO_PAGE_X = 0x94;
    static constexpr std::uint8_t STY_ABSOLUTE    = 0x8C;

    /// Register Increment Opcode
    static constexpr std::uint8_t INX = 0xE8;
    static constexpr std::uint8_t INY = 0xC8;
    static constexpr std::uint8_t DEX = 0xCA;
    static constexpr std::uint8_t DEY = 0x88;

    /// Jump Opcodes
    static constexpr std::uint8_t JMP_ABSOLUTE = 0x4C;
    static constexpr std::uint8_t JMP_INDIRECT = 0x6C;
    static constexpr std::uint8_t JMP_JSR = 0x20;
    static constexpr std::uint8_t JMP_RTS = 0x60;
    static constexpr std::uint8_t JMP_BRK = 0x00;
    static constexpr std::uint8_t JMP_RTI = 0x40;

    /// AND (bitwise AND with Accumulator)
    static constexpr std::uint8_t AND_IMMEDIATE   = 0x29;
    static constexpr std::uint8_t AND_ZERO_PAGE   = 0x25;
    static constexpr std::uint8_t AND_ZERO_PAGE_X = 0x35;
    static constexpr std::uint8_t AND_ABSOLUTE    = 0x2D;
    static constexpr std::uint8_t AND_ABSOLUTE_X  = 0x3D;
    static constexpr std::uint8_t AND_ABSOLUTE_Y  = 0x39;
    static constexpr std::uint8_t AND_INDIRECT_X  = 0x21;
    static constexpr std::uint8_t AND_INDIRECT_Y  = 0x31;

    /// ORA (bitwise OR with Accumulator)
    static constexpr std::uint8_t ORA_IMMEDIATE   = 0x09;
    static constexpr std::uint8_t ORA_ZERO_PAGE   = 0x05;
    static constexpr std::uint8_t ORA_ZERO_PAGE_X = 0x15;
    static constexpr std::uint8_t ORA_ABSOLUTE    = 0x0D;
    static constexpr std::uint8_t ORA_ABSOLUTE_X  = 0x1D;
    static constexpr std::uint8_t ORA_ABSOLUTE_Y  = 0x19;
    static constexpr std::uint8_t ORA_INDIRECT_X  = 0x01;
    static constexpr std::uint8_t ORA_INDIRECT_Y  = 0x11;

    /// EOR
    static constexpr std::uint8_t EOR_IMMEDIATE   = 0x49;
    static constexpr std::uint8_t EOR_ZERO_PAGE   = 0x45;
    static constexpr std::uint8_t EOR_ZERO_PAGE_X = 0x55;
    static constexpr std::uint8_t EOR_ABSOLUTE    = 0x4D;
    static constexpr std::uint8_t EOR_ABSOLUTE_X  = 0x5D;
    static constexpr std::uint8_t EOR_ABSOLUTE_Y  = 0x59;
    static constexpr std::uint8_t EOR_INDIRECT_X  = 0x41;
    static constexpr std::uint8_t EOR_INDIRECT_Y  = 0x51;

    /// Stack Opcodes
    static constexpr std::uint8_t PHA = 0x48;
    static constexpr std::uint8_t PHP = 0x08;
    static constexpr std::uint8_t PLA = 0x68;
    static constexpr std::uint8_t PLP = 0x28;

    /// Flag Opcodes
    static constexpr std::uint8_t CLC = 0x18;
    static constexpr std::uint8_t SEC = 0x38;
    static constexpr std::uint8_t CLI = 0x58;
    static constexpr std::uint8_t SEI = 0x78;
    static constexpr std::uint8_t CLV = 0xB8;
    static constexpr std::uint8_t CLD = 0xD8;
    static constexpr std::uint8_t SED = 0xF8;

    /// Register Transfer Opcodes
    static constexpr std::uint8_t TAX = 0xAA;
    static constexpr std::uint8_t TAY = 0xA8;
    static constexpr std::uint8_t TXA = 0x8A;
    static constexpr std::uint8_t TYA = 0x98;
    static constexpr std::uint8_t TSX = 0xBA;
    static constexpr std::uint8_t TXS = 0x9A;

    /// CMP Opcodes
    static  constexpr std::uint8_t CMP_IMMEDIATE   = 0xC9;
    static  constexpr std::uint8_t CMP_ZERO_PAGE   = 0xC5;
    static  constexpr std::uint8_t CMP_ZERO_PAGE_X = 0xD5;
    static  constexpr std::uint8_t CMP_ABSOLUTE    = 0xCD;
    static  constexpr std::uint8_t CMP_ABSOLUTE_X  = 0xDD;
    static  constexpr std::uint8_t CMP_ABSOLUTE_Y  = 0xD9;
    static  constexpr std::uint8_t CMP_INDIRECT_X  = 0xC1;
    static  constexpr std::uint8_t CMP_INDIRECT_Y  = 0xD1;
    /// CPX Opcodes
    static  constexpr std::uint8_t CPX_IMMEDIATE   = 0xE0;
    static  constexpr std::uint8_t CPX_ZERO_PAGE   = 0xE4;
    static  constexpr std::uint8_t CPX_ABSOLUTE    = 0xEC;
    /// CPY Opcodes
    static  constexpr std::uint8_t CPY_IMMEDIATE   = 0xC0;
    static  constexpr std::uint8_t CPY_ZERO_PAGE   = 0xC4;
    static  constexpr std::uint8_t CPY_ABSOLUTE    = 0xCC;

    /// Branch Opcodes
    static constexpr std::uint8_t BEQ = 0xF0;
    static constexpr std::uint8_t BNE = 0xD0;
    static constexpr std::uint8_t BCS = 0xB0;
    static constexpr std::uint8_t BCC = 0x90;
    static constexpr std::uint8_t BMI = 0x30;
    static constexpr std::uint8_t BPL = 0x10;
    static constexpr std::uint8_t BVS = 0x70;
    static constexpr std::uint8_t BVC = 0x50;

    /// Shift Opcodes
    /// ASL
    static constexpr std::uint8_t ASL_ACCUMULATOR = 0x0A;
    static constexpr std::uint8_t ASL_ZERO_PAGE   = 0x06;
    static constexpr std::uint8_t ASL_ZERO_PAGE_X = 0x16;
    static constexpr std::uint8_t ASL_ABSOLUTE    = 0x0E;
    static constexpr std::uint8_t ASL_ABSOLUTE_X  = 0x1E;
    /// LSR
    static constexpr std::uint8_t LSR_ACCUMULATOR = 0x4A;
    static constexpr std::uint8_t LSR_ZERO_PAGE   = 0x46;
    static constexpr std::uint8_t LSR_ZERO_PAGE_X = 0x56;
    static constexpr std::uint8_t LSR_ABSOLUTE    = 0x4E;
    static constexpr std::uint8_t LSR_ABSOLUTE_X  = 0x5E;
    /// ROL
    static constexpr std::uint8_t ROL_ACCUMULATOR = 0x2A;
    static constexpr std::uint8_t ROL_ZERO_PAGE   = 0x26;
    static constexpr std::uint8_t ROL_ZERO_PAGE_X = 0x36;
    static constexpr std::uint8_t ROL_ABSOLUTE    = 0x2E;
    static constexpr std::uint8_t ROL_ABSOLUTE_X  = 0x3E;
    /// ROR
    static constexpr std::uint8_t ROR_ACCUMULATOR = 0x6A;
    static constexpr std::uint8_t ROR_ZERO_PAGE   = 0x66;
    static constexpr std::uint8_t ROR_ZERO_PAGE_X = 0x76;
    static constexpr std::uint8_t ROR_ABSOLUTE    = 0x6E;
    static constexpr std::uint8_t ROR_ABSOLUTE_X  = 0x7E;

    /// ADC
    static constexpr std::uint8_t ADC_IMMEDIATE   = 0x69;
    static constexpr std::uint8_t ADC_ZERO_PAGE   = 0x65;
    static constexpr std::uint8_t ADC_ZERO_PAGE_X = 0x75;
    static constexpr std::uint8_t ADC_ABSOLUTE    = 0x6D;
    static constexpr std::uint8_t ADC_ABSOLUTE_X  = 0x7D;
    static constexpr std::uint8_t ADC_ABSOLUTE_Y  = 0x79;
    static constexpr std::uint8_t ADC_INDIRECT_X  = 0x61;
    static constexpr std::uint8_t ADC_INDIRECT_Y  = 0x71;

    /// SBC
    static constexpr std::uint8_t SBC_IMMEDIATE   = 0xE9;
    static constexpr std::uint8_t SBC_ZERO_PAGE   = 0xE5;
    static constexpr std::uint8_t SBC_ZERO_PAGE_X = 0xF5;
    static constexpr std::uint8_t SBC_ABSOLUTE    = 0xED;
    static constexpr std::uint8_t SBC_ABSOLUTE_X  = 0xFD;
    static constexpr std::uint8_t SBC_ABSOLUTE_Y  = 0xF9;
    static constexpr std::uint8_t SBC_INDIRECT_X  = 0xE1;
    static constexpr std::uint8_t SBC_INDIRECT_Y  = 0xF1;

    /// Misc Opcodes
    /// INC
    static constexpr std::uint8_t INC_ZERO_PAGE   = 0xE6;
    static constexpr std::uint8_t INC_ZERO_PAGE_X = 0xF6;
    static constexpr std::uint8_t INC_ABSOLUTE    = 0xEE;
    static constexpr std::uint8_t INC_ABSOLUTE_X  = 0xFE;
    /// DEC
    static constexpr std::uint8_t DEC_ZERO_PAGE   = 0xC6;
    static constexpr std::uint8_t DEC_ZERO_PAGE_X = 0xD6;
    static constexpr std::uint8_t DEC_ABSOLUTE    = 0xCE;
    static constexpr std::uint8_t DEC_ABSOLUTE_X  = 0xDE;
    /// BIT
    static constexpr std::uint8_t BIT_ZERO_PAGE   = 0x24;
    static constexpr std::uint8_t BIT_ABSOLUTE    = 0x2C;
    /// NOP
    static constexpr std::uint8_t NOP             = 0xEA;

    static constexpr std::array<int, 256> CYCLES = [] {
        std::array<int, 256> cycles{};
        // LDA Cycles
        cycles[LDA_IMMEDIATE]   = 2;
        cycles[LDA_ZERO_PAGE]   = 3;
        cycles[LDA_ZERO_PAGE_X] = 4;
        cycles[LDA_ABSOLUTE]    = 4;
        cycles[LDA_ABSOLUTE_X]  = 4;
        cycles[LDA_ABSOLUTE_Y]  = 4;
        cycles[LDA_INDIRECT_X]  = 6;
        cycles[LDA_INDIRECT_Y]  = 5;
        // LDX Cycles
        cycles[LDX_IMMEDIATE]   = 2;
        cycles[LDX_ZERO_PAGE]   = 3;
        cycles[LDX_ZERO_PAGE_Y] = 4;
        cycles[LDX_ABSOLUTE]    = 4;
        cycles[LDX_ABSOLUTE_Y]  = 4;
        // LDY Cycles
        cycles[LDY_IMMEDIATE]   = 2;
        cycles[LDY_ZERO_PAGE]   = 3;
        cycles[LDY_ZERO_PAGE_X] = 4;
        cycles[LDY_ABSOLUTE]    = 4;
        cycles[LDY_ABSOLUTE_X]  = 4;
        // STA Cycles
        cycles[STA_ZERO_PAGE]   = 3;
        cycles[STA_ZERO_PAGE_X] = 4;
        cycles[STA_ABSOLUTE]    = 4;
        cycles[STA_ABSOLUTE_X]  = 5;
        cycles[STA_ABSOLUTE_Y]  = 5;
        cycles[STA_INDIRECT_X]  = 6;
        cycles[STA_INDIRECT_Y]  = 6;
        // STX Cycles
        cycles[STX_ZERO_PAGE]   = 3;
        cycles[STX_ZERO_PAGE_Y] = 4;
        cycles[STX_ABSOLUTE]    = 4;
        // STY Cycles
        cycles[STY_ZERO_PAGE]   = 3;
        cycles[STY_ZERO_PAGE_X] = 4;
        cycles[STY_ABSOLUTE]    = 4;
        // Register Increment Cycles
        cycles[INX] = 2;
        cycles[INY] = 2;
        cycles[DEX] = 2;
        cycles[DEY] = 2;
        // Jump Cycles
        cycles[JMP_ABSOLUTE] = 3;
        cycles[JMP_INDIRECT] = 5;
        cycles[JMP_JSR] = 6;
        cycles[JMP_RTS] = 6;
        cycles[JMP_BRK] = 7;
        cycles[JMP_RTI] = 6;
        // AND Cycles
        cycles[AND_IMMEDIATE]   = 2;
        cycles[AND_ZERO_PAGE]   = 3;
        cycles[AND_ZERO_PAGE_X] = 4;
        cycles[AND_ABSOLUTE]    = 4;
        cycles[AND_ABSOLUTE_X]  = 4;
        cycles[AND_ABSOLUTE_Y]  = 4;
        cycles[AND_INDIRECT_X]  = 6;
        cycles[AND_INDIRECT_Y]  = 5;
        // ORA Cycles
        cycles[ORA_IMMEDIATE]   = 2;
        cycles[ORA_ZERO_PAGE]   = 3;
        cycles[ORA_ZERO_PAGE_X] = 4;
        cycles[ORA_ABSOLUTE]    = 4;
        cycles[ORA_ABSOLUTE_X]  = 4;
        cycles[ORA_ABSOLUTE_Y]  = 4;
        cycles[ORA_INDIRECT_X]  = 6;
        cycles[ORA_INDIRECT_Y]  = 5;
        cycles[EOR_IMMEDIATE]   = 2;
        cycles[EOR_ZERO_PAGE]   = 3;
        cycles[EOR_ZERO_PAGE_X] = 4;
        cycles[EOR_ABSOLUTE]    = 4;
        cycles[EOR_ABSOLUTE_X]  = 4;
        cycles[EOR_ABSOLUTE_Y]  = 4;
        cycles[EOR_INDIRECT_X]  = 6;
        cycles[EOR_INDIRECT_Y]  = 5;
        // Stack Cycles
        cycles[PHA] = 3;
        cycles[PHP] = 3;
        cycles[PLA] = 4;
        cycles[PLP] = 4;
        // Flag Cycles
        cycles[CLC] = 2;
        cycles[SEC] = 2;
        cycles[CLI] = 2;
        cycles[SEI] = 2;
        cycles[CLV] = 2;
        cycles[CLD] = 2;
        cycles[SED] = 2;
        // Register Transfer Cycles
        cycles[TAX] = 2;
        cycles[TAY] = 2;
        cycles[TXA] = 2;
        cycles[TYA] = 2;
        cycles[TSX] = 2;
        cycles[TXS] = 2;
        // CPM Cycles
        cycles[CMP_IMMEDIATE]   = 2;
        cycles[CMP_ZERO_PAGE]   = 3;
        cycles[CMP_ZERO_PAGE_X] = 4;
        cycles[CMP_ABSOLUTE]    = 4;
        cycles[CMP_ABSOLUTE_X]  = 4;
        cycles[CMP_ABSOLUTE_Y]  = 4;
        cycles[CMP_INDIRECT_X]  = 6;
        cycles[CMP_INDIRECT_Y]  = 5;
        // CPX Cycles
        cycles[CPX_IMMEDIATE]   = 2;
        cycles[CPX_ZERO_PAGE]   = 3;
        cycles[CPX_ABSOLUTE]    = 4;
        // CPY Cycles
        cycles[CPY_IMMEDIATE]   = 2;
        cycles[CPY_ZERO_PAGE]   = 3;
        cycles[CPY_ABSOLUTE]    = 4;
        // Branch Cycles
        cycles[BEQ] = 2;
        cycles[BNE] = 2;
        cycles[BCS] = 2;
        cycles[BCC] = 2;
        cycles[BMI] = 2;
        cycles[BPL] = 2;
        cycles[BVS] = 2;
        cycles[BVC] = 2;
        // Shift Cycles
        cycles[ASL_ACCUMULATOR] = 2;
        cycles[ASL_ZERO_PAGE]   = 5;
        cycles[ASL_ZERO_PAGE_X] = 6;
        cycles[ASL_ABSOLUTE]    = 6;
        cycles[ASL_ABSOLUTE_X]  = 7;
        cycles[LSR_ACCUMULATOR] = 2;
        cycles[LSR_ZERO_PAGE]   = 5;
        cycles[LSR_ZERO_PAGE_X] = 6;
        cycles[LSR_ABSOLUTE]    = 6;
        cycles[LSR_ABSOLUTE_X]  = 7;
        cycles[ROL_ACCUMULATOR] = 2;
        cycles[ROL_ZERO_PAGE]   = 5;
        cycles[ROL_ZERO_PAGE_X] = 6;
        cycles[ROL_ABSOLUTE]    = 6;
        cycles[ROL_ABSOLUTE_X]  = 7;
        cycles[ROR_ACCUMULATOR] = 2;
        cycles[ROR_ZERO_PAGE]   = 5;
        cycles[ROR_ZERO_PAGE_X] = 6;
        cycles[ROR_ABSOLUTE]    = 6;
        cycles[ROR_ABSOLUTE_X]  = 7;
        // Arithmetic Cycles
        cycles[ADC_IMMEDIATE]   = 2;
        cycles[ADC_ZERO_PAGE]   = 3;
        cycles[ADC_ZERO_PAGE_X] = 4;
        cycles[ADC_ABSOLUTE]    = 4;
        cycles[ADC_ABSOLUTE_X]  = 4;
        cycles[ADC_ABSOLUTE_Y]  = 4;
        cycles[ADC_INDIRECT_X]  = 6;
        cycles[ADC_INDIRECT_Y]  = 5;
        cycles[SBC_IMMEDIATE]   = 2;
        cycles[SBC_ZERO_PAGE]   = 3;
        cycles[SBC_ZERO_PAGE_X] = 4;
        cycles[SBC_ABSOLUTE]    = 4;
        cycles[SBC_ABSOLUTE_X]  = 4;
        cycles[SBC_ABSOLUTE_Y]  = 4;
        cycles[SBC_INDIRECT_X]  = 6;
        cycles[SBC_INDIRECT_Y]  = 5;
        // Misc cycles
        cycles[INC_ZERO_PAGE]   = 5;
        cycles[INC_ZERO_PAGE_X] = 6;
        cycles[INC_ABSOLUTE]    = 6;
        cycles[INC_ABSOLUTE_X]  = 7;
        cycles[DEC_ZERO_PAGE]   = 5;
        cycles[DEC_ZERO_PAGE_X] = 6;
        cycles[DEC_ABSOLUTE]    = 6;
        cycles[DEC_ABSOLUTE_X]  = 7;
        cycles[BIT_ZERO_PAGE]   = 3;
        cycles[BIT_ABSOLUTE]    = 4;
        cycles[NOP]             = 2;
        return cycles;
    }();
};

} // nes

#endif //NES_EMULATOR_CPP_OPCODES_H