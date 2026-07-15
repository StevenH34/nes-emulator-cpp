#include "Cpu.h"
#include "Opcodes.h"

namespace nes {

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
        case Opcodes::INY:
            Iny();
            break;
        case Opcodes::DEX:
            Dex();
            break;
        case Opcodes::DEY:
            Dey();
            break;
        // Jumps
        case Opcodes::JMP_ABSOLUTE:
            JmpAbsolute();
            break;
        case Opcodes::JMP_INDIRECT:
            JmpIndirect();
            break;
        case Opcodes::JMP_JSR:
            Jsr();
            break;
        case Opcodes::JMP_RTS:
            Rts();
            break;
        case Opcodes::JMP_BRK:
            Brk();
            break;
        case Opcodes::JMP_RTI:
            Rti();
            break;
        // Register Transfers
        case Opcodes::TAX:
            Tax();
            break;
        case Opcodes::TAY:
            Tay();
            break;
        case Opcodes::TXA:
            Txa();
            break;
        case Opcodes::TYA:
            Tya();
            break;
        case Opcodes::TSX:
            Tsx();
            break;
        case Opcodes::TXS:
            Txs();
            break;
        // Flags
        case Opcodes::CLC:
            Clc();
            break;
        case Opcodes::SEC:
            Sec();
            break;
        case Opcodes::CLI:
            Cli();
            break;
        case Opcodes::SEI:
            Sei();
            break;
        case Opcodes::CLD:
            Cld();
            break;
        case Opcodes::SED:
            Sed();
            break;
        case Opcodes::CLV:
            Clv();
            break;
        // Stack
        case Opcodes::PHA:
            Pha();
            break;
        case Opcodes::PLA:
            Pla();
            break;
        case Opcodes::PHP:
            Php();
            break;
        case Opcodes::PLP:
            Plp();
            break;
        // AND
        case Opcodes::AND_IMMEDIATE:
            AndImmediate();
            break;
        case Opcodes::AND_ZERO_PAGE:
            AndZeroPage();
            break;
        case Opcodes::AND_ZERO_PAGE_X:
            AndZeroPageX();
            break;
        case Opcodes::AND_ABSOLUTE:
            AndAbsolute();
            break;
        case Opcodes::AND_ABSOLUTE_X:
            AndAbsoluteX();
            break;
        case Opcodes::AND_ABSOLUTE_Y:
            AndAbsoluteY();
            break;
        case Opcodes::AND_INDIRECT_X:
            AndIndirectX();
            break;
        case Opcodes::AND_INDIRECT_Y:
            AndIndirectY();
            break;
        // ORA
        case Opcodes::ORA_IMMEDIATE:
            OraImmediate();
            break;
        case Opcodes::ORA_ZERO_PAGE:
            OraZeroPage();
            break;
        case Opcodes::ORA_ZERO_PAGE_X:
            OraZeroPageX();
            break;
        case Opcodes::ORA_ABSOLUTE:
            OraAbsolute();
            break;
        case Opcodes::ORA_ABSOLUTE_X:
            OraAbsoluteX();
            break;
        case Opcodes::ORA_ABSOLUTE_Y:
            OraAbsoluteY();
            break;
        case Opcodes::ORA_INDIRECT_X:
            OraIndirectX();
            break;
        case Opcodes::ORA_INDIRECT_Y:
            OraIndirectY();
            break;
        // EOR
        case Opcodes::EOR_IMMEDIATE:
            EorImmediate();
            break;
        case Opcodes::EOR_ZERO_PAGE:
            EorZeroPage();
            break;
        case Opcodes::EOR_ZERO_PAGE_X:
            EorZeroPageX();
            break;
        case Opcodes::EOR_ABSOLUTE:
            EorAbsolute();
            break;
        case Opcodes::EOR_ABSOLUTE_X:
            EorAbsoluteX();
            break;
        case Opcodes::EOR_ABSOLUTE_Y:
            EorAbsoluteY();
            break;
        case Opcodes::EOR_INDIRECT_X:
            EorIndirectX();
            break;
        case Opcodes::EOR_INDIRECT_Y:
            EorIndirectY();
            break;
        // CMP
        case Opcodes::CMP_IMMEDIATE:
            CmpImmediate();
            break;
        case Opcodes::CMP_ZERO_PAGE:
            CmpZeroPage();
            break;
        case Opcodes::CMP_ZERO_PAGE_X:
            CmpZeroPageX();
            break;
        case Opcodes::CMP_ABSOLUTE:
            CmpAbsolute();
            break;
        case Opcodes::CMP_ABSOLUTE_X:
            CmpAbsoluteX();
            break;
        case Opcodes::CMP_ABSOLUTE_Y:
            CmpAbsoluteY();
            break;
        case Opcodes::CMP_INDIRECT_X:
            CmpIndirectX();
            break;
        case Opcodes::CMP_INDIRECT_Y:
            CmpIndirectY();
            break;
        // CPX
        case Opcodes::CPX_IMMEDIATE:
            CpxImmediate();
            break;
        case Opcodes::CPX_ZERO_PAGE:
            CpxZeroPage();
            break;
        case Opcodes::CPX_ABSOLUTE:
            CpxAbsolute();
            break;
        // CPY
        case Opcodes::CPY_IMMEDIATE:
            CpyImmediate();
            break;
        case Opcodes::CPY_ZERO_PAGE:
            CpyZeroPage();
            break;
        case Opcodes::CPY_ABSOLUTE:
            CpyAbsolute();
            break;
        // Branch Instructions
        case Opcodes::BEQ:
            Beq();
            break;
        case Opcodes::BNE:
            Bne();
            break;
        case Opcodes::BCS:
            Bcs();
            break;
        case Opcodes::BCC:
            Bcc();
            break;
        case Opcodes::BMI:
            Bmi();
            break;
        case Opcodes::BPL:
            Bpl();
            break;
        case Opcodes::BVS:
            Bvs();
            break;
        case Opcodes::BVC:
            Bvc();
            break;
        // ADC
        case Opcodes::ADC_IMMEDIATE:
            AdcImmediate();
            break;
        case Opcodes::ADC_ZERO_PAGE:
            AdcZeroPage();
            break;
        case Opcodes::ADC_ZERO_PAGE_X:
            AdcZeroPageX();
            break;
        case Opcodes::ADC_ABSOLUTE:
            AdcAbsolute();
            break;
        case Opcodes::ADC_ABSOLUTE_X:
            AdcAbsoluteX();
            break;
        case Opcodes::ADC_ABSOLUTE_Y:
            AdcAbsoluteY();
            break;
        case Opcodes::ADC_INDIRECT_X:
            AdcIndirectX();
            break;
        case Opcodes::ADC_INDIRECT_Y:
            AdcIndirectY();
            break;
        // SBC
        case Opcodes::SBC_IMMEDIATE:
            SbcImmediate();
            break;
        case Opcodes::SBC_ZERO_PAGE:
            SbcZeroPage();
            break;
        case Opcodes::SBC_ZERO_PAGE_X:
            SbcZeroPageX();
            break;
        case Opcodes::SBC_ABSOLUTE:
            SbcAbsolute();
            break;
        case Opcodes::SBC_ABSOLUTE_X:
            SbcAbsoluteX();
            break;
        case Opcodes::SBC_ABSOLUTE_Y:
            SbcAbsoluteY();
            break;
        case Opcodes::SBC_INDIRECT_X:
            SbcIndirectX();
            break;
        case Opcodes::SBC_INDIRECT_Y:
            SbcIndirectY();
            break;
        // ASL
        case Opcodes::ASL_ACCUMULATOR:
            AslAccumulator();
            break;
        case Opcodes::ASL_ZERO_PAGE:
            AslZeroPage();
            break;
        case Opcodes::ASL_ZERO_PAGE_X:
            AslZeroPageX();
            break;
        case Opcodes::ASL_ABSOLUTE:
            AslAbsolute();
            break;
        case Opcodes::ASL_ABSOLUTE_X:
            AslAbsoluteX();
            break;
        // LSR
        case Opcodes::LSR_ACCUMULATOR:
            LsrAccumulator();
            break;
        case Opcodes::LSR_ZERO_PAGE:
            LsrZeroPage();
            break;
        case Opcodes::LSR_ZERO_PAGE_X:
            LsrZeroPageX();
            break;
        case Opcodes::LSR_ABSOLUTE:
            LsrAbsolute();
            break;
        case Opcodes::LSR_ABSOLUTE_X:
            LsrAbsoluteX();
            break;
        // ROL
        case Opcodes::ROL_ACCUMULATOR:
            RolAccumulator();
            break;
        case Opcodes::ROL_ZERO_PAGE:
            RolZeroPage();
            break;
        case Opcodes::ROL_ZERO_PAGE_X:
            RolZeroPageX();
            break;
        case Opcodes::ROL_ABSOLUTE:
            RolAbsolute();
            break;
        case Opcodes::ROL_ABSOLUTE_X:
            RolAbsoluteX();
            break;
        // ROR
        case Opcodes::ROR_ACCUMULATOR:
            RorAccumulator();
            break;
        case Opcodes::ROR_ZERO_PAGE:
            RorZeroPage();
            break;
        case Opcodes::ROR_ZERO_PAGE_X:
            RorZeroPageX();
            break;
        case Opcodes::ROR_ABSOLUTE:
            RorAbsolute();
            break;
        case Opcodes::ROR_ABSOLUTE_X:
            RorAbsoluteX();
            break;
        // INC
        case Opcodes::INC_ZERO_PAGE:
            IncZeroPage();
            break;
        case Opcodes::INC_ZERO_PAGE_X:
            IncZeroPageX();
            break;
        case Opcodes::INC_ABSOLUTE:
            IncAbsolute();
            break;
        case Opcodes::INC_ABSOLUTE_X:
            IncAbsoluteX();
            break;
        // DEC
        case Opcodes::DEC_ZERO_PAGE:
            DecZeroPage();
            break;
        case Opcodes::DEC_ZERO_PAGE_X:
            DecZeroPageX();
            break;
        case Opcodes::DEC_ABSOLUTE:
            DecAbsolute();
            break;
        case Opcodes::DEC_ABSOLUTE_X:
            DecAbsoluteX();
            break;
        // BIT
        case Opcodes::BIT_ABSOLUTE:
            BitAbsolute();
            break;
        case Opcodes::BIT_ZERO_PAGE:
            BitZeroPage();
            break;
        // NOP
        case Opcodes::NOP:
            Nop();
            break;
        default:
            throw UnknownOpcode(opcode);
    }

    return Opcodes::CYCLES[opcode];
}

} // nes
