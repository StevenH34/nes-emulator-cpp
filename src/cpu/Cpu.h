#ifndef NES_EMULATOR_CPP_CPU_H
#define NES_EMULATOR_CPP_CPU_H

#include <cstdint>
#include <format>
#include <stdexcept>
#include <string>

#include "Bus.h"

namespace nes {

class UnknownOpcode : public std::runtime_error {
public:
    explicit UnknownOpcode(const std::uint8_t opcode)
        : std::runtime_error(std::format("Unknown opcode: 0x{:02X}", opcode)) {}
};

class Cpu {
public:
    explicit Cpu(Bus& bus);

    void PrintDebugging();
    [[nodiscard]] std::string StatusString() const;
    void Reset();
    [[nodiscard]] int Step();
    [[nodiscard]] std::uint8_t FetchByte();
    [[nodiscard]] std::uint8_t ReadByte(std::uint16_t address) const;
    void WriteByte(std::uint16_t address, std::uint8_t value);

    [[nodiscard]] std::uint8_t GetAccumulator() const { return accumulator_; }
    [[nodiscard]] std::uint8_t GetXRegister() const { return x_register_;};
    [[nodiscard]] std::uint8_t GetYRegister() const { return y_register_; }
    [[nodiscard]] std::uint8_t GetStatusRegister() const { return status_register_; }
    [[nodiscard]] std::uint8_t GetStackPointer() const { return stack_pointer_; }
    [[nodiscard]] std::uint16_t GetProgramCounter() const { return program_counter_; }

    // Test helpers for driving register state directly
    void SetXRegister(const std::uint8_t value) { x_register_ = value; }
    void SetYRegister(const std::uint8_t value) { y_register_ = value; }
    void SetProgramCounter(const std::uint16_t value) { program_counter_ = value; }

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

    /// STA (STore Accumulator)
    /// Affects flags: none
    void StaZeroPage();
    void StaZeroPageX();
    void StaAbsolute();
    void StaAbsoluteX();
    void StaAbsoluteY();
    void StaIndirectX();
    void StaIndirectY();

    /// LDA (LoaD Accumulator)
    /// Affects flags: N Z
    void Lda(std::uint8_t value);
    void LdaImmediate();
    void LdaZeroPage();
    void LdaZeroPageX();
    void LdaAbsolute();
    void LdaAbsoluteX();
    void LdaAbsoluteY();
    void LdaIndirectX();
    void LdaIndirectY();

    /// LDX (LoaD X register)
    void Ldx(std::uint8_t value);
    void LdxImmediate();
    void LdxZeroPage();
    void LdxZeroPageY();
    void LdxAbsolute();
    void LdxAbsoluteY();

    /// LDY (LoaD Y register)
    void Ldy(std::uint8_t value);
    void LdyImmediate();
    void LdyZeroPage();
    void LdyZeroPageX();
    void LdyAbsolute();
    void LdyAbsoluteX();

    /// STX (STore X register)
    void StxZeroPage();
    void StxZeroPageY();
    void StxAbsolute();

    /// STY (STore Y register)
    void StyZeroPage();
    void StyZeroPageX();
    void StyAbsolute();

    /// Register Increment Instruction
    void Inx();
    void Iny();
    void Dex();
    void Dey();

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

    /// Flag Methods
    void SetFlag(StatusFlag flag, bool is_on);
    [[nodiscard]] bool IsFlagSet(std::uint8_t mask) const;
    void SetZFlag(std::uint8_t register_value); // Zero Flag
    void SetNFlag(std::uint8_t register_value); // Negative Flag
    void SetCFlag(bool is_on);  // Carry Flag
    void SetVFlag(bool is_on);  // Overflow Flag

    /**
     * Flag (Processor Status) Instructions
     *
     * CLC (CLear Carry)
     * SEC (SEt Carry)
     * CLI (CLear Interrupt)
     * SEI (SEt Interrupt)
     * CLV (CLear oVerflow)
     * CLD (CLear Decimal)
     * SED (SEt Decimal)
     */
    void Clc();
    void Sec();
    void Cli();
    void Sei();
    void Cld();
    void Sed();
    void Clv();
    
    /// Branch Instructions
    void BranchIf(bool condition);
    void Beq(); // Z == 1, branch if equal
    void Bne(); // Z == 0, branch if not equal
    void Bcs(); // C == 1, branch if carry set
    void Bcc(); // C == 0, branch if carry clear
    void Bmi(); // N == 1, branch if minus
    void Bpl(); // N == 0, branch if plus
    void Bvs(); // V == 1, branch if overflow set
    void Bvc(); // V == 0, branch if overflow clear

    /// Jump Instructions
    void JmpAbsolute();
    void JmpIndirect();
    void Jsr(); // Jump to subroutine
    void Rts(); // Return from subroutine
    void Brk(); // Saves the full CPU state (Program Counter and flags)
    void Rti(); // Restores Program Counter and flags from the stack

    /// Stack Methods
    // Lives at Page 1: $0100 - $01FF
    void StackPushByte(std::uint8_t value);
    std::uint8_t StackPullByte();
    void StackPushWord(std::uint16_t value);
    std::uint16_t StackPullWord();

    /**
     * Stack Instructions
     *
     * PHA (PusH Accumulator) - Push accumulator to the stack
     * PLA (PuLl Accumulator) - Pull from stack to accumulator
     * PHP (PusH Processor status) - Push status register to the stack
     * PLP (PuLl Processor status) - Pull from stack to status register
     */
    void Pha();
    void Pla();
    void Php();
    void Plp();

    /// Comparison Instructions
    void Compare(std::uint8_t register_value, std::uint8_t operand);

    /// CMP (CoMPare accumulator)
    /// Affects Flags: N Z C
    void CmpImmediate();
    void CmpZeroPage();
    void CmpZeroPageX();
    void CmpAbsolute();
    void CmpAbsoluteX();
    void CmpAbsoluteY();
    void CmpIndirectX();
    void CmpIndirectY();

    /// CPX (ComPare X register)
    /// Affects Flags: N Z C
    void CpxImmediate();
    void CpxZeroPage();
    void CpxAbsolute();

    /// CPY
    void CpyImmediate();
    void CpyZeroPage();
    void CpyAbsolute();

    /// Shift Instructions
    /// ASL (Arithmetic Shift Left)
    /// Affects Flags: N Z C
    std::uint8_t Asl(std::uint8_t value);
    void AslAccumulator();
    void AslZeroPage();
    void AslZeroPageX();
    void AslAbsolute();
    void AslAbsoluteX();

    /// LSR (Logical Shift Right)
    /// Affects Flags: N Z C
    std::uint8_t Lsr(std::uint8_t value);
    void LsrAccumulator();
    void LsrZeroPage();
    void LsrZeroPageX();
    void LsrAbsolute();
    void LsrAbsoluteX();

    /// ROL (ROtate Left)
    /// Affects Flags: N Z C
    std::uint8_t Rol(std::uint8_t value);
    void RolAccumulator();
    void RolZeroPage();
    void RolZeroPageX();
    void RolAbsolute();
    void RolAbsoluteX();

    /// ROR (ROtate Right)
    /// Affects Flags: N Z C
    std::uint8_t Ror(std::uint8_t value);
    void RorAccumulator();
    void RorZeroPage();
    void RorZeroPageX();
    void RorAbsolute();
    void RorAbsoluteX();

    /// ADC (Add with Carry)
    /// Affects flags: N V Z C
    void Adc(std::uint8_t value);
    void AdcImmediate();
    void AdcZeroPage();
    void AdcZeroPageX();
    void AdcAbsolute();
    void AdcAbsoluteX();
    void AdcAbsoluteY();
    void AdcIndirectX();
    void AdcIndirectY();

    /// SBC (Subtract with Carry)
    /// Affects Flags: N V Z C
    void Sbc(std::uint8_t value);
    void SbcImmediate();
    void SbcZeroPage();
    void SbcZeroPageX();
    void SbcAbsolute();
    void SbcAbsoluteX();
    void SbcAbsoluteY();
    void SbcIndirectX();
    void SbcIndirectY();

    /// Register Instructions
    /// Affect Flags: N Z
    /// These instructions are implied mode, have a length of 1 byte, and require 2 cycles
    void Tax(); // Transfer register A to X
    void Tay(); // Transfer register A to Y
    void Txa(); // Transfer register X to A
    void Tya(); // Transfer register Y to A
    void Tsx(); // Transfer Stack Pointer to register X
    void Txs(); // Transfer X to Stack Pointer

    /// AND (bitwise AND with Accumulator)
    /// Affects flags: N Z
    void AndImmediate();
    void AndZeroPage();
    void AndZeroPageX();
    void AndAbsolute();
    void AndAbsoluteX();
    void AndAbsoluteY();
    void AndIndirectX();
    void AndIndirectY();

    /// ORA (bitwise OR with Accumulator)
    /// Affects flags: N Z
    void OraImmediate();
    void OraZeroPage();
    void OraZeroPageX();
    void OraAbsolute();
    void OraAbsoluteX();
    void OraAbsoluteY();
    void OraIndirectX();
    void OraIndirectY();

    /// EOR (bitwise Exclusive OR)
    void EorImmediate();
    void EorZeroPage();
    void EorZeroPageX();
    void EorAbsolute();
    void EorAbsoluteX();
    void EorAbsoluteY();
    void EorIndirectX();
    void EorIndirectY();

    /// Misc Instructions
    /// INC (INCrement memory)
    /// Affects Flags: N Z
    void IncZeroPage();
    void IncZeroPageX();
    void IncAbsolute();
    void IncAbsoluteX();

    /// DEC (DECrement memory)
    /// Affects Flags: N Z
    void DecZeroPage();
    void DecZeroPageX();
    void DecAbsolute();
    void DecAbsoluteX();

    /**
     * BIT (test BITs)
     * Affects Flags: N V Z
     * Used to test if one or more bits are set in a target memory location.
     * The mask pattern in A is ANDed with the value in memory to set or clear the zero flag,
     * but the result is not kept.
     * Bits 7 and 6 of the value from memory are copied into the N and V flags.
     */
    void BitZeroPage();
    void BitAbsolute();

    /// NOP (No OPeration)
    void Nop();

private:
    Bus& bus_;

    std::uint8_t STATUS_INIT_ = static_cast<uint8_t>(StatusFlag::U) | static_cast<uint8_t>(StatusFlag::I);

    /// Stack
    std::uint16_t STACK_BASE_ = 0x0100;
    std::uint8_t STACK_INIT_ = 0xFD;

    /// CPU Registers
    std::uint8_t accumulator_ = 0;                  // Accumulator
    std::uint8_t x_register_ = 0;                   // X Register
    std::uint8_t y_register_ = 0;                   // Y Register
    std::uint8_t stack_pointer_ = STACK_INIT_;      // Stack Pointer
    std::uint8_t status_register_ = STATUS_INIT_;   // Status Register (flags)
    std::uint16_t program_counter_ = 0;             // Program Counter

    std::uint8_t MAX_8_BIT_UINT_ = 0xFF; // Maximum number that fits in a byte (8 bits): 255

    /// Interrupt Vectors
    std::uint16_t NMI_VECTOR_ = 0xFFFA; // The PPU finished drawing a frame (Non-Maskable Interrupt)
    std::uint16_t RESET_VECTOR_ = 0xFFFC;
    std::uint16_t IRQ_VECTOR_ = 0xFFFE; // Hardware interrupt
};

} // nes

#endif //NES_EMULATOR_CPP_CPU_H
