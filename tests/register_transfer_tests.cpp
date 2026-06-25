#include "../doctest.h"

#include "../src/Bus.h"
#include "../src/Cpu.h"

TEST_CASE("Tax copies the accumulator into the X register and updates the Zero and Negative flags") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x00);
    cpu.Tax();
    CHECK(cpu.GetXRegister() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc"); // Zero flag set

    cpu.Lda(0x80);
    cpu.Tax();
    CHECK(cpu.GetXRegister() == 0x80);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Negative flag set

    cpu.Lda(0x01);
    cpu.Tax();
    CHECK(cpu.GetXRegister() == 0x01);
    CHECK(cpu.StatusString() == "nvUbdIzc"); // both flags clear
    CHECK(cpu.GetAccumulator() == 0x01); // accumulator left unchanged
}

TEST_CASE("Tay copies the accumulator into the Y register and updates the Zero and Negative flags") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x00);
    cpu.Tay();
    CHECK(cpu.GetYRegister() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc"); // Zero flag set

    cpu.Lda(0x80);
    cpu.Tay();
    CHECK(cpu.GetYRegister() == 0x80);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Negative flag set

    cpu.Lda(0x01);
    cpu.Tay();
    CHECK(cpu.GetYRegister() == 0x01);
    CHECK(cpu.StatusString() == "nvUbdIzc"); // both flags clear
    CHECK(cpu.GetAccumulator() == 0x01); // accumulator left unchanged
}

TEST_CASE("Txa copies the X register into the accumulator and updates the Zero and Negative flags") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.SetXRegister(0x00);
    cpu.Txa();
    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc"); // Zero flag set

    cpu.SetXRegister(0x80);
    cpu.Txa();
    CHECK(cpu.GetAccumulator() == 0x80);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Negative flag set

    cpu.SetXRegister(0x01);
    cpu.Txa();
    CHECK(cpu.GetAccumulator() == 0x01);
    CHECK(cpu.StatusString() == "nvUbdIzc"); // both flags clear
    CHECK(cpu.GetXRegister() == 0x01); // X register left unchanged
}

TEST_CASE("Tya copies the Y register into the accumulator and updates the Zero and Negative flags") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.SetYRegister(0x00);
    cpu.Tya();
    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc"); // Zero flag set

    cpu.SetYRegister(0x80);
    cpu.Tya();
    CHECK(cpu.GetAccumulator() == 0x80);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Negative flag set

    cpu.SetYRegister(0x01);
    cpu.Tya();
    CHECK(cpu.GetAccumulator() == 0x01);
    CHECK(cpu.StatusString() == "nvUbdIzc"); // both flags clear
    CHECK(cpu.GetYRegister() == 0x01); // Y register left unchanged
}

TEST_CASE("Tsx copies the stack pointer into the X register and updates the Zero and Negative flags") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Tsx(); // stack pointer starts at 0xFD (bit 7 set)
    CHECK(cpu.GetXRegister() == 0xFD);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Negative flag set

    // Drive the stack pointer down to 0x00 by pushing 0xFD bytes (starts at 0xFD).
    for (int i = 0; i < 0xFD; ++i) {
        cpu.StackPushByte(static_cast<std::uint8_t>(i));
    }

    cpu.Tsx();
    CHECK(cpu.GetXRegister() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc"); // Zero flag set
}

TEST_CASE("Txs copies the X register into the stack pointer without affecting flags") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.SetFlag(nes::Cpu::StatusFlag::N, true);
    cpu.SetFlag(nes::Cpu::StatusFlag::Z, true);
    const auto status_before = cpu.StatusString();

    cpu.SetXRegister(0x10);
    cpu.Txs();

    CHECK(cpu.StatusString() == status_before); // Txs does not touch any flags

    // Confirm the stack pointer actually moved by observing where the next push lands.
    cpu.StackPushByte(0x99);
    CHECK(bus.ReadCpu(0x0110) == 0x99);
}