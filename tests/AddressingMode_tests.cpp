#include "../doctest.h"

#include "../Bus.h"
#include "../Cpu.h"

TEST_CASE("AddressAbsolute combines low and high bytes in little-endian order") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x34); // low byte
    bus.WriteCpu(0x01, 0x12); // high byte

    CHECK(cpu.AddressAbsolute() == 0x1234);
}

TEST_CASE("AddressZeroPageX resolves base + X within the zero page") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetXRegister(0x05);

    CHECK(cpu.AddressZeroPageX() == 0x15);
}

TEST_CASE("AddressZeroPageX wraps within the zero page instead of crossing into page 1") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xFF); // base operand
    cpu.SetXRegister(0x02);

    CHECK(cpu.AddressZeroPageX() == 0x01);
}

TEST_CASE("AddressZeroPageY resolves base + Y within the zero page") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x20); // base operand
    cpu.SetYRegister(0x03);

    CHECK(cpu.AddressZeroPageY() == 0x23);
}

TEST_CASE("AddressZeroPageY wraps within the zero page instead of crossing into page 1") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xFF); // base operand
    cpu.SetYRegister(0x05);

    CHECK(cpu.AddressZeroPageY() == 0x04);
}

TEST_CASE("AddressAbsoluteX adds X to a 16-bit base address") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x02); // high byte, base address = 0x0200
    cpu.SetXRegister(0x10);

    CHECK(cpu.AddressAbsoluteX() == 0x0210);
}

TEST_CASE("AddressAbsoluteX wraps across the 16-bit address space") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xFF); // low byte
    bus.WriteCpu(0x01, 0xFF); // high byte, base address = 0xFFFF
    cpu.SetXRegister(0x01);

    CHECK(cpu.AddressAbsoluteX() == 0x0000);
}

TEST_CASE("AddressAbsoluteY adds Y to a 16-bit base address") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // low byte
    bus.WriteCpu(0x01, 0x03); // high byte, base address = 0x0300
    cpu.SetYRegister(0x20);

    CHECK(cpu.AddressAbsoluteY() == 0x0320);
}

TEST_CASE("AddressAbsoluteY wraps across the 16-bit address space") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xFF); // low byte
    bus.WriteCpu(0x01, 0xFF); // high byte, base address = 0xFFFF
    cpu.SetYRegister(0x02);

    CHECK(cpu.AddressAbsoluteY() == 0x0001);
}

TEST_CASE("AddressRelative adds a positive signed offset to the Program Counter") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x05); // offset = +5, fetched from PC 0x0000

    // After fetching the offset byte the Program Counter is 0x0001
    CHECK(cpu.AddressRelative() == 0x0006);
}

TEST_CASE("AddressRelative subtracts a negative signed offset from the Program Counter") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xA9); // LDA Immediate opcode
    bus.WriteCpu(0x01, 0x10); // LDA value, advances PC to 0x0002
    bus.WriteCpu(0x02, 0xFE); // offset = -2

    CHECK(cpu.Step() == 2); // executes LDA Immediate, PC is now 0x0002
    // After fetching the offset byte the Program Counter is 0x0003
    CHECK(cpu.AddressRelative() == 0x0001);
}

TEST_CASE("AddressIndirect reads the target address from the pointer location") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x00); // pointer low byte
    bus.WriteCpu(0x01, 0x03); // pointer high byte, pointer address = 0x0300
    bus.WriteCpu(0x0300, 0x34); // target low byte
    bus.WriteCpu(0x0301, 0x12); // target high byte

    CHECK(cpu.AddressIndirect() == 0x1234);
}

TEST_CASE("AddressIndirect emulates the page-boundary bug when the pointer's low byte is 0xFF") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xFF); // pointer low byte
    bus.WriteCpu(0x01, 0x02); // pointer high byte, pointer address = 0x02FF
    bus.WriteCpu(0x02FF, 0x80); // target low byte
    bus.WriteCpu(0x0200, 0x01); // target high byte read from the wrapped page start
    bus.WriteCpu(0x0300, 0xFF); // would be (wrongly) read if the page didn't wrap

    CHECK(cpu.AddressIndirect() == 0x0180);
}

TEST_CASE("AddressIndirectX adds X to the zero page base, then reads a 16-bit pointer") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x10); // base operand
    cpu.SetXRegister(0x05);
    bus.WriteCpu(0x15, 0x78); // pointer low byte
    bus.WriteCpu(0x16, 0x56); // pointer high byte

    CHECK(cpu.AddressIndirectX() == 0x5678);
}

TEST_CASE("AddressIndirectX wraps the high-byte pointer fetch within the zero page") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xF0); // base operand, also read back as the wrapped high byte
    cpu.SetXRegister(0x0F); // base + X = 0xFF
    bus.WriteCpu(0xFF, 0x34); // pointer low byte

    CHECK(cpu.AddressIndirectX() == 0xF034);
}

TEST_CASE("AddressIndirectY reads a zero page pointer, then adds Y to it") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0x20); // base operand, used directly as the zero page pointer
    bus.WriteCpu(0x20, 0x00); // pointer low byte
    bus.WriteCpu(0x21, 0x02); // pointer high byte, base address = 0x0200
    cpu.SetYRegister(0x05);

    CHECK(cpu.AddressIndirectY() == 0x0205);
}

TEST_CASE("AddressIndirectY wraps the high-byte pointer fetch within the zero page") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    bus.WriteCpu(0x00, 0xFF); // base operand, also read back as the wrapped high byte
    bus.WriteCpu(0xFF, 0x80); // pointer low byte
    cpu.SetYRegister(0x01);

    CHECK(cpu.AddressIndirectY() == 0xFF81);
}