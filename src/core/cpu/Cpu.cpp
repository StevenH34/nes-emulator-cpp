#include "Cpu.h"

#include <format>
#include <print>

namespace nes {

Cpu::Cpu(Bus& bus) : bus_(bus) {}

void Cpu::PrintDebugging() const {
  std::println("A={:02X}, X={:02X}, Y={:02X}, SP={:02X}, PC={:04X} [{}]", accumulator_, x_register_, y_register_,
               stack_pointer_, program_counter_, StatusString());
}

std::string Cpu::StatusString() const {
  const auto s = status_register_;
  std::string output;
  output.reserve(8);

  output += (s >> 7 & 1) != 0 ? 'N' : 'n';
  output += (s >> 6 & 1) != 0 ? 'V' : 'v';
  output += (s >> 5 & 1) != 0 ? 'U' : 'u';
  output += (s >> 4 & 1) != 0 ? 'B' : 'b';
  output += (s >> 3 & 1) != 0 ? 'D' : 'd';
  output += (s >> 2 & 1) != 0 ? 'I' : 'i';
  output += (s >> 1 & 1) != 0 ? 'Z' : 'z';
  output += (s >> 0 & 1) != 0 ? 'C' : 'c';

  return output;
}

void Cpu::Reset() {
  const uint8_t low = ReadByte(RESET_VECTOR_);
  const uint8_t high = ReadByte(RESET_VECTOR_ + 1);
  program_counter_ = static_cast<uint16_t>(high << 8) | static_cast<uint16_t>(low);
}

// Reads byte at the current Program Counter, then increments Program Counter
uint8_t Cpu::FetchByte() {
  const auto value = ReadByte(program_counter_);
  program_counter_ = static_cast<uint16_t>(program_counter_ + 1);
  return value;
}

uint8_t Cpu::ReadByte(const uint16_t address) const { return bus_.ReadCpu(address); }

void Cpu::WriteByte(const uint16_t address, const uint8_t value) const { bus_.WriteCpu(address, value); }

/// Addressing Modes
// Read a byte and convert it to a 16-bit address
uint16_t Cpu::AddressZeroPage() { return FetchByte(); }

// Zero Page + X offset
uint16_t Cpu::AddressZeroPageX() {
  const auto base = FetchByte();
  return static_cast<uint8_t>(base + x_register_);
}

// Zero Page + Y offset
uint16_t Cpu::AddressZeroPageY() {
  const auto base = FetchByte();
  return static_cast<uint8_t>(base + y_register_);
}

// Read two bytes then combine them
uint16_t Cpu::AddressAbsolute() {
  const auto low_byte = FetchByte();
  const auto high_byte = FetchByte();
  // Move high_byte because of little endian
  return static_cast<uint16_t>(high_byte << 8) | static_cast<uint16_t>(low_byte);
}

// Absolute + X offset
uint16_t Cpu::AddressAbsoluteX() { return static_cast<uint16_t>(AddressAbsolute() + x_register_); }

// Absolute + Y offset
uint16_t Cpu::AddressAbsoluteY() { return static_cast<uint16_t>(AddressAbsolute() + y_register_); }

// Relative addressing is used for branch instructions.
// It provides a signed offset (-128 to 127) to the current Program Counter.
uint16_t Cpu::AddressRelative() {
  const auto offest = static_cast<int8_t>(FetchByte());
  return static_cast<uint16_t>(program_counter_ + offest);
}

// Indirect: JMP ($nnnn)
// Reads a 16-bit address from the pointer location, then jumps to that address.
uint16_t Cpu::AddressIndirect() {
  const uint16_t pointer_address = AddressAbsolute();
  const uint8_t low_byte = ReadByte(pointer_address);

  // Emulate the 6502-page-boundary bug
  const uint16_t high_byte_address = (pointer_address & 0xFF) == 0xFF
                                         ? pointer_address & 0xFF00 // Wrap around to the start of the page $xx00
                                         : pointer_address + 1; // Normal case

  const uint8_t high_byte = ReadByte(high_byte_address);
  return static_cast<uint16_t>(high_byte << 8) | static_cast<uint16_t>(low_byte);
}

// Indexed Indirect: LDA ($nn,X)
// Adds X to Zero-Page address, then reads a 16-bit pointer address
uint16_t Cpu::AddressIndirectX() {
  const uint8_t base = FetchByte();
  const auto pointer = static_cast<uint8_t>(base + x_register_);
  const uint8_t low_byte = ReadByte(pointer);
  // The high-byte pointer also wraps within the zero page
  const uint8_t high_byte = ReadByte(static_cast<uint8_t>(pointer + 1));
  return static_cast<uint16_t>(high_byte << 8) | static_cast<uint16_t>(low_byte);
}

uint16_t Cpu::AddressIndirectY() {
  const uint8_t base = FetchByte();
  const uint8_t low_byte = ReadByte(base);
  // The high-byte pointer wraps within the zero page
  const uint8_t high_byte = ReadByte(static_cast<uint8_t>(base + 1));
  const uint16_t address = static_cast<uint16_t>(high_byte << 8) | static_cast<uint16_t>(low_byte);
  return static_cast<uint16_t>(address + y_register_);
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
void Cpu::Lda(const uint8_t value) {
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
void Cpu::Ldx(const uint8_t value) {
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
void Cpu::Ldy(const uint8_t value) {
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

/// Register Increment Instructions
void Cpu::Inx() {
  x_register_ = static_cast<uint8_t>(x_register_ + 1);
  SetZFlag(x_register_);
  SetNFlag(x_register_);
}

void Cpu::Iny() {
  y_register_ = static_cast<uint8_t>(y_register_ + 1);
  SetZFlag(y_register_);
  SetNFlag(y_register_);
}

void Cpu::Dex() {
  x_register_ = static_cast<uint8_t>(x_register_ - 1);
  SetZFlag(x_register_);
  SetNFlag(x_register_);
}

void Cpu::Dey() {
  y_register_ = static_cast<uint8_t>(y_register_ - 1);
  SetZFlag(y_register_);
  SetNFlag(y_register_);
}

/// Flag Methods
void Cpu::SetFlag(const StatusFlag flag, const bool is_on) {
  const auto mask = static_cast<uint8_t>(flag);
  if (is_on) {
    // Use OR to turn the bit on.
    status_register_ |= mask;
  } else {
    // Use AND with inverted mask to turn the bit off.
    status_register_ &= static_cast<uint8_t>(~mask);
  }
}

bool Cpu::IsFlagSet(const uint8_t mask) const { return (status_register_ & mask) != 0; }

// Turns the Zero Flag on when the Most Significant Bit (bit 7) is 1.
// This means the number is negative in two's complement.
void Cpu::SetZFlag(const uint8_t register_value) { SetFlag(StatusFlag::Z, register_value == 0); }

void Cpu::SetNFlag(const uint8_t register_value) {
  SetFlag(StatusFlag::N,
          (register_value >> 7 & 1) == 1); // Most Significant Bit
};

void Cpu::SetCFlag(const bool is_on) { SetFlag(StatusFlag::C, is_on); }

void Cpu::SetVFlag(const bool is_on) { SetFlag(StatusFlag::V, is_on); }

/// Flag Instructions
void Cpu::Clc() { SetFlag(StatusFlag::C, false); }

void Cpu::Sec() { SetFlag(StatusFlag::C, true); }

void Cpu::Cli() { SetFlag(StatusFlag::I, false); }

void Cpu::Sei() { SetFlag(StatusFlag::I, true); }

void Cpu::Cld() { SetFlag(StatusFlag::D, false); }

void Cpu::Sed() { SetFlag(StatusFlag::D, true); }

void Cpu::Clv() { SetFlag(StatusFlag::V, false); }

/// Branch Instructions
void Cpu::BranchIf(const bool condition) {
  const auto target = AddressRelative();
  if (condition)
    program_counter_ = target;
}

void Cpu::Beq() { BranchIf(IsFlagSet(static_cast<uint8_t>(StatusFlag::Z))); }

void Cpu::Bne() { BranchIf(!IsFlagSet(static_cast<uint8_t>(StatusFlag::Z))); }

void Cpu::Bcs() { BranchIf(IsFlagSet(static_cast<uint8_t>(StatusFlag::C))); }

void Cpu::Bcc() { BranchIf(!IsFlagSet(static_cast<uint8_t>(StatusFlag::C))); }

void Cpu::Bmi() { BranchIf(IsFlagSet(static_cast<uint8_t>(StatusFlag::N))); }

void Cpu::Bpl() { BranchIf(!IsFlagSet(static_cast<uint8_t>(StatusFlag::N))); }

void Cpu::Bvs() { BranchIf(IsFlagSet(static_cast<uint8_t>(StatusFlag::V))); }

void Cpu::Bvc() { BranchIf(!IsFlagSet(static_cast<uint8_t>(StatusFlag::V))); }

/// Jump Instructions
void Cpu::JmpAbsolute() {
  // Reads 16-bit address and sets PC to it
  program_counter_ = AddressAbsolute();
}

void Cpu::JmpIndirect() {
  // Reads 16-bit address where destination is stored and sets PC to it
  program_counter_ = AddressIndirect();
}

void Cpu::Jsr() {
  // Read the destination address
  const uint16_t target = AddressAbsolute();
  // Push PC - 1 onto the stack
  StackPushWord(program_counter_ - 1);
  // Set PC to the destination address - the jump
  program_counter_ = target;
}

void Cpu::Rts() {
  // Pull return address from the stack and increment by 1
  program_counter_ = StackPullWord() + 1;
}

void Cpu::Brk() {
  // Push Program Counter + 1 to the stack - the return address
  StackPushWord(program_counter_ + 1);
  // Save the flags - push Status Register to the stack
  StackPushByte(status_register_ | static_cast<uint8_t>(StatusFlag::B) | static_cast<uint8_t>(StatusFlag::U));
  // Set I Flag to disable interrupts
  SetFlag(StatusFlag::I, true);
  // Read address from IRQ/BRK and jump to address
  const uint8_t low_byte = ReadByte(IRQ_VECTOR_);
  const uint8_t high_byte = ReadByte(IRQ_VECTOR_ + 1);
  program_counter_ = static_cast<uint16_t>(high_byte << 8) | static_cast<uint16_t>(low_byte);
}

void Cpu::Rti() {
  // Restore flags - clear B, force U
  status_register_ = (StackPullByte() & ~static_cast<uint8_t>(StatusFlag::B)) | static_cast<uint8_t>(StatusFlag::U);
  // Restore Program Counter
  program_counter_ = StackPullWord();
}

/// Stack Methods
void Cpu::StackPushByte(const uint8_t value) {
  // Write current value at stack address then decrement stack pointer
  WriteByte(STACK_BASE_ | static_cast<uint16_t>(stack_pointer_), value);
  stack_pointer_ = static_cast<uint8_t>(stack_pointer_ - 1);
}

uint8_t Cpu::StackPullByte() {
  // Decrements stack pointer then reads address
  stack_pointer_ = static_cast<uint8_t>(stack_pointer_ + 1);
  return ReadByte(STACK_BASE_ | static_cast<uint16_t>(stack_pointer_));
}

void Cpu::StackPushWord(const uint16_t value) {
  StackPushByte(static_cast<uint8_t>(value >> 8)); // High byte
  StackPushByte(static_cast<uint8_t>(value & 0xFF)); // Low byte
}

uint16_t Cpu::StackPullWord() {
  const uint8_t low_byte = StackPullByte();
  const uint8_t high_byte = StackPullByte();
  return static_cast<uint16_t>(high_byte << 8) | static_cast<uint16_t>(low_byte);
}

/// Stack Instructions
void Cpu::Pha() { StackPushByte(accumulator_); }

void Cpu::Pla() {
  accumulator_ = StackPullByte();
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::Php() {
  StackPushByte(status_register_ | static_cast<uint8_t>(StatusFlag::B) | static_cast<uint8_t>(StatusFlag::U));
}

void Cpu::Plp() {
  status_register_ = (StackPullByte() & ~static_cast<uint8_t>(StatusFlag::B)) | static_cast<uint8_t>(StatusFlag::U);
}

/// Comparison Instructions
void Cpu::Compare(const uint8_t register_value, const uint8_t operand) {
  const uint8_t result = static_cast<uint8_t>(register_value - operand);
  // If register >= operand, not borrow, so C = 1, else C = 0
  SetCFlag(register_value >= operand);
  // If result = 0, Z = 1
  SetZFlag(result);
  // Bit 7 of the result
  SetNFlag(result);
}

/// CMP
void Cpu::CmpImmediate() {
  const auto value = FetchByte();
  Compare(accumulator_, value);
}

void Cpu::CmpZeroPage() {
  const auto value = ReadByte(AddressZeroPage());
  Compare(accumulator_, value);
}

void Cpu::CmpZeroPageX() {
  const auto value = ReadByte(AddressZeroPageX());
  Compare(accumulator_, value);
}

void Cpu::CmpAbsolute() {
  const auto value = ReadByte(AddressAbsolute());
  Compare(accumulator_, value);
}

void Cpu::CmpAbsoluteX() {
  const auto value = ReadByte(AddressAbsoluteX());
  Compare(accumulator_, value);
}

void Cpu::CmpAbsoluteY() {
  const auto value = ReadByte(AddressAbsoluteY());
  Compare(accumulator_, value);
}

void Cpu::CmpIndirectX() {
  const auto value = ReadByte(AddressIndirectX());
  Compare(accumulator_, value);
}

void Cpu::CmpIndirectY() {
  const auto value = ReadByte(AddressIndirectY());
  Compare(accumulator_, value);
}

/// CPX
void Cpu::CpxImmediate() {
  const auto value = FetchByte();
  Compare(x_register_, value);
}

void Cpu::CpxZeroPage() {
  const auto value = ReadByte(AddressZeroPage());
  Compare(x_register_, value);
}

void Cpu::CpxAbsolute() {
  const auto value = ReadByte(AddressAbsolute());
  Compare(x_register_, value);
}

/// CPY
void Cpu::CpyImmediate() {
  const auto value = FetchByte();
  Compare(y_register_, value);
}

void Cpu::CpyZeroPage() {
  const auto value = ReadByte(AddressZeroPage());
  Compare(y_register_, value);
}

void Cpu::CpyAbsolute() {
  const auto value = ReadByte(AddressAbsolute());
  Compare(y_register_, value);
}

/// Shift Instructions
/**
 * ASL (Arithmetic Shift Left), moves all bits one position to the left.
 * Bit 7 goes to Carry flag
 */
uint8_t Cpu::Asl(const uint8_t value) {
  SetCFlag((value >> 7 & 1) == 1); // 7-bit falls off and goes to the C flag
  const uint8_t result = static_cast<uint8_t>(value << 1);
  SetZFlag(result);
  SetNFlag(result);
  return result;
}

void Cpu::AslAccumulator() { accumulator_ = Asl(accumulator_); }

void Cpu::AslZeroPage() {
  const uint16_t address = AddressZeroPage();
  const uint8_t value = ReadByte(address);
  WriteByte(address, Asl(value));
}

void Cpu::AslZeroPageX() {
  const uint16_t address = AddressZeroPageX();
  const uint8_t value = ReadByte(address);
  WriteByte(address, Asl(value));
}

void Cpu::AslAbsolute() {
  const uint16_t address = AddressAbsolute();
  const uint8_t value = ReadByte(address);
  WriteByte(address, Asl(value));
}

void Cpu::AslAbsoluteX() {
  const uint16_t address = AddressAbsoluteX();
  const uint8_t value = ReadByte(address);
  WriteByte(address, Asl(value));
}

/**
 * LSR (Logical Shift Right), moves all bits one position to the right.
 * Bit 0 goes to the Carry flag
 */
uint8_t Cpu::Lsr(const uint8_t value) {
  SetCFlag((value & 1) == 1); // 0-bit falls off and goes to the C flag
  const uint8_t result = value >> 1;
  SetZFlag(result);
  SetNFlag(result);
  return result;
}

void Cpu::LsrAccumulator() { accumulator_ = Lsr(accumulator_); }

void Cpu::LsrZeroPage() {
  const uint16_t address = AddressZeroPage();
  const uint8_t value = ReadByte(address);
  WriteByte(address, Lsr(value));
}

void Cpu::LsrZeroPageX() {
  const uint16_t address = AddressZeroPageX();
  const uint8_t value = ReadByte(address);
  WriteByte(address, Lsr(value));
}

void Cpu::LsrAbsolute() {
  const uint16_t address = AddressAbsolute();
  const uint8_t value = ReadByte(address);
  WriteByte(address, Lsr(value));
}

void Cpu::LsrAbsoluteX() {
  const uint16_t address = AddressAbsoluteX();
  const uint8_t value = ReadByte(address);
  WriteByte(address, Lsr(value));
}

/**
 * ROL (ROtate Left)
 * Bit 7 goes to Carry, old Carry goes to bit 0
 */
uint8_t Cpu::Rol(const uint8_t value) {
  const uint8_t carry_in = IsFlagSet(static_cast<uint8_t>(StatusFlag::C)) ? 1 : 0;
  SetCFlag((value >> 7 & 1) == 1); // 7-bit falls off and goes to the C flag
  const uint8_t result = static_cast<uint8_t>((value << 1) | carry_in);
  SetZFlag(result);
  SetNFlag(result);
  return result;
}

void Cpu::RolAccumulator() { accumulator_ = Rol(accumulator_); }

void Cpu::RolZeroPage() {
  const uint16_t address = AddressZeroPage();
  const uint8_t value = ReadByte(address);
  WriteByte(address, Rol(value));
}

void Cpu::RolZeroPageX() {
  const uint16_t address = AddressZeroPageX();
  const uint8_t value = ReadByte(address);
  WriteByte(address, Rol(value));
}

void Cpu::RolAbsolute() {
  const uint16_t address = AddressAbsolute();
  const uint8_t value = ReadByte(address);
  WriteByte(address, Rol(value));
}

void Cpu::RolAbsoluteX() {
  const uint16_t address = AddressAbsoluteX();
  const uint8_t value = ReadByte(address);
  WriteByte(address, Rol(value));
}

/**
 * ROR (ROtate Right)
 * Bit 0 goes to Carry, old Carry goes to bit 7
 */

uint8_t Cpu::Ror(const uint8_t value) {
  const uint8_t carry = IsFlagSet(static_cast<uint8_t>(StatusFlag::C)) ? 0x80 : 0x00;
  SetCFlag((value >> 0 & 1) == 1); // 0-bit falls off and goes to the C flag
  const uint8_t result = value >> 1 | carry;
  SetZFlag(result);
  SetNFlag(result);
  return result;
}

void Cpu::RorAccumulator() { accumulator_ = Ror(accumulator_); }

void Cpu::RorZeroPage() {
  const uint16_t address = AddressZeroPage();
  const uint8_t value = ReadByte(address);
  WriteByte(address, Ror(value));
}

void Cpu::RorZeroPageX() {
  const uint16_t address = AddressZeroPageX();
  const uint8_t value = ReadByte(address);
  WriteByte(address, Ror(value));
}

void Cpu::RorAbsolute() {
  const uint16_t address = AddressAbsolute();
  const uint8_t value = ReadByte(address);
  WriteByte(address, Ror(value));
}

void Cpu::RorAbsoluteX() {
  const uint16_t address = AddressAbsoluteX();
  const uint8_t value = ReadByte(address);
  WriteByte(address, Ror(value));
}

/// ADC (Add with Carry)
void Cpu::Adc(const uint8_t value) {
  // Get current value of the carry flag
  const uint16_t carry_in = IsFlagSet(static_cast<uint8_t>(StatusFlag::C)) ? 1 : 0;
  const uint16_t sum = static_cast<uint16_t>(accumulator_) + static_cast<uint16_t>(value) + carry_in;
  const auto result = static_cast<uint8_t>(sum);

  // Check for unsigned overflow
  SetCFlag(sum > MAX_8_BIT_UINT_);
  // Check for signed overflow
  // XOR both operands. If bit 7 is 0, both operands have the same sign.
  // Else both operands have different signs
  // ~(@a ^ value): If bit 7 is 0, inverse this to 1 for true
  // (@a ^ result): True if both signs are different
  // AND both to combine results
  // & 0x80: Check only the signed bit (bit 7)
  // If all 3 are true, inputs had the same sign, but result flipped the sign -
  // this is overflow Two positives means it's a negative Two negatives means
  // it's a positive If one is positive and one is negative overflow cannot
  // happen
  SetVFlag((~(accumulator_ ^ value) & (accumulator_ ^ result) & 0x80) != 0);

  accumulator_ = result;

  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::AdcImmediate() {
  const auto value = FetchByte();
  Adc(value);
}

void Cpu::AdcZeroPage() {
  const auto value = ReadByte(AddressZeroPage());
  Adc(value);
}

void Cpu::AdcZeroPageX() {
  const auto value = ReadByte(AddressZeroPageX());
  Adc(value);
}

void Cpu::AdcAbsolute() {
  const auto value = ReadByte(AddressAbsolute());
  Adc(value);
}

void Cpu::AdcAbsoluteX() {
  const auto value = ReadByte(AddressAbsoluteX());
  Adc(value);
}

void Cpu::AdcAbsoluteY() {
  const auto value = ReadByte(AddressAbsoluteY());
  Adc(value);
}

void Cpu::AdcIndirectX() {
  const auto value = ReadByte(AddressIndirectX());
  Adc(value);
}

void Cpu::AdcIndirectY() {
  const auto value = ReadByte(AddressIndirectY());
  Adc(value);
}

/// SBC (Subtract with Carry)
void Cpu::Sbc(const uint8_t value) {
  Adc(static_cast<uint8_t>(~value)); // Subtracting is the same as adding the one's complement
}

void Cpu::SbcImmediate() {
  const auto value = FetchByte();
  Sbc(value);
}

void Cpu::SbcZeroPage() {
  const auto value = ReadByte(AddressZeroPage());
  Sbc(value);
}

void Cpu::SbcZeroPageX() {
  const auto value = ReadByte(AddressZeroPageX());
  Sbc(value);
}

void Cpu::SbcAbsolute() {
  const auto value = ReadByte(AddressAbsolute());
  Sbc(value);
}

void Cpu::SbcAbsoluteX() {
  const auto value = ReadByte(AddressAbsoluteX());
  Sbc(value);
}

void Cpu::SbcAbsoluteY() {
  const auto value = ReadByte(AddressAbsoluteY());
  Sbc(value);
}

void Cpu::SbcIndirectX() {
  const auto value = ReadByte(AddressIndirectX());
  Sbc(value);
}

void Cpu::SbcIndirectY() {
  const auto value = ReadByte(AddressIndirectY());
  Sbc(value);
}

/// Register Instructions
void Cpu::Tax() {
  x_register_ = accumulator_;
  SetZFlag(x_register_);
  SetNFlag(x_register_);
}

void Cpu::Tay() {
  y_register_ = accumulator_;
  SetZFlag(y_register_);
  SetNFlag(y_register_);
}

void Cpu::Txa() {
  accumulator_ = x_register_;
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::Tya() {
  accumulator_ = y_register_;
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}
void Cpu::Tsx() {
  x_register_ = stack_pointer_;
  SetZFlag(x_register_);
  SetNFlag(x_register_);
}

void Cpu::Txs() { stack_pointer_ = x_register_; }

/// AND
void Cpu::AndImmediate() {
  accumulator_ &= FetchByte();
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::AndZeroPage() {
  accumulator_ &= ReadByte(AddressZeroPage());
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::AndZeroPageX() {
  accumulator_ &= ReadByte(AddressZeroPageX());
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::AndAbsolute() {
  accumulator_ &= ReadByte(AddressAbsolute());
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::AndAbsoluteX() {
  accumulator_ &= ReadByte(AddressAbsoluteX());
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::AndAbsoluteY() {
  accumulator_ &= ReadByte(AddressAbsoluteY());
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::AndIndirectX() {
  accumulator_ &= ReadByte(AddressIndirectX());
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::AndIndirectY() {
  accumulator_ &= ReadByte(AddressIndirectY());
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

/// ORA
void Cpu::OraImmediate() {
  accumulator_ |= FetchByte();
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::OraZeroPage() {
  accumulator_ |= ReadByte(AddressZeroPage());
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::OraZeroPageX() {
  accumulator_ |= ReadByte(AddressZeroPageX());
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::OraAbsolute() {
  accumulator_ |= ReadByte(AddressAbsolute());
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::OraAbsoluteX() {
  accumulator_ |= ReadByte(AddressAbsoluteX());
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::OraAbsoluteY() {
  accumulator_ |= ReadByte(AddressAbsoluteY());
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::OraIndirectX() {
  accumulator_ |= ReadByte(AddressIndirectX());
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::OraIndirectY() {
  accumulator_ |= ReadByte(AddressIndirectY());
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

/// EOR
void Cpu::EorImmediate() {
  accumulator_ ^= FetchByte();
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::EorZeroPage() {
  accumulator_ ^= ReadByte(AddressZeroPage());
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::EorZeroPageX() {
  accumulator_ ^= ReadByte(AddressZeroPageX());
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::EorAbsolute() {
  accumulator_ ^= ReadByte(AddressAbsolute());
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::EorAbsoluteX() {
  accumulator_ ^= ReadByte(AddressAbsoluteX());
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::EorAbsoluteY() {
  accumulator_ ^= ReadByte(AddressAbsoluteY());
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::EorIndirectX() {
  accumulator_ ^= ReadByte(AddressIndirectX());
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

void Cpu::EorIndirectY() {
  accumulator_ ^= ReadByte(AddressIndirectY());
  SetZFlag(accumulator_);
  SetNFlag(accumulator_);
}

/// Misc Instructions
/**
 * INC (INCrement memory)
 * Adds one to the value held at a specified memory location setting the zero
 * and negative flags as appropriate.
 */
void Cpu::IncZeroPage() {
  const uint16_t address = AddressZeroPage();
  const uint8_t value = static_cast<uint8_t>(ReadByte(address) + 1);
  WriteByte(address, value);
  SetZFlag(value);
  SetNFlag(value);
}

void Cpu::IncZeroPageX() {
  const uint16_t address = AddressZeroPageX();
  const uint8_t value = static_cast<uint8_t>(ReadByte(address) + 1);
  WriteByte(address, value);
  SetZFlag(value);
  SetNFlag(value);
}

void Cpu::IncAbsolute() {
  const uint16_t address = AddressAbsolute();
  const uint8_t value = static_cast<uint8_t>(ReadByte(address) + 1);
  WriteByte(address, value);
  SetZFlag(value);
  SetNFlag(value);
}

void Cpu::IncAbsoluteX() {
  const uint16_t address = AddressAbsoluteX();
  const uint8_t value = static_cast<uint8_t>(ReadByte(address) + 1);
  WriteByte(address, value);
  SetZFlag(value);
  SetNFlag(value);
}

/**
 * DEC (DECrement memory)
 * Subtracts one from the value held at a specified memory location setting the
 * zero and negative flags as appropriate.
 */
void Cpu::DecZeroPage() {
  const uint16_t address = AddressZeroPage();
  const uint8_t value = static_cast<uint8_t>(ReadByte(address) - 1);
  WriteByte(address, value);
  SetZFlag(value);
  SetNFlag(value);
}

void Cpu::DecZeroPageX() {
  const uint16_t address = AddressZeroPageX();
  const uint8_t value = static_cast<uint8_t>(ReadByte(address) - 1);
  WriteByte(address, value);
  SetZFlag(value);
  SetNFlag(value);
}

void Cpu::DecAbsolute() {
  const uint16_t address = AddressAbsolute();
  const uint8_t value = static_cast<uint8_t>(ReadByte(address) - 1);
  WriteByte(address, value);
  SetZFlag(value);
  SetNFlag(value);
}

void Cpu::DecAbsoluteX() {
  const uint16_t address = AddressAbsoluteX();
  const uint8_t value = static_cast<uint8_t>(ReadByte(address) - 1);
  WriteByte(address, value);
  SetZFlag(value);
  SetNFlag(value);
}

/// BIT
void Cpu::BitZeroPage() {
  const uint8_t value = ReadByte(AddressZeroPage());
  SetZFlag(accumulator_ & value);
  SetNFlag(value);
  SetVFlag((value >> 6 & 1) == 1); // Bit 6 goes to V flag
}

void Cpu::BitAbsolute() {
  const uint8_t value = ReadByte(AddressAbsolute());
  SetZFlag(accumulator_ & value);
  SetNFlag(value);
  SetVFlag((value >> 6 & 1) == 1);
}

/// NOP
void Cpu::Nop() {
  // Do nothing
}

void Cpu::Nmi() {
  StackPushWord(program_counter_);
  StackPushByte((status_register_ | static_cast<uint8_t>(StatusFlag::U)) & ~static_cast<uint8_t>(StatusFlag::B));
  SetFlag(StatusFlag::I, true);
  const uint8_t low_byte = ReadByte(NMI_VECTOR_);
  const uint8_t high_byte = ReadByte(NMI_VECTOR_ + 1);
  program_counter_ = static_cast<uint16_t>(high_byte << 8) | static_cast<uint16_t>(low_byte);
}

} // namespace nes
