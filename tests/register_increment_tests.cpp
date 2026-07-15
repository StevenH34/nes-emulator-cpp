#include "doctest.h"

#include "../src/Bus.h"
#include "TestBus.h"
#include "../src/cpu/Cpu.h"

TEST_CASE("Inx increments the X register and updates the Zero and Negative flags") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.SetXRegister(0x09);
    cpu.Inx();
    CHECK(cpu.GetXRegister() == 0x0A);
    CHECK(cpu.StatusString() == "nvUbdIzc"); // both flags clear

    cpu.SetXRegister(0xFF);
    cpu.Inx();
    CHECK(cpu.GetXRegister() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc"); // wraps to 0x00, Zero flag set

    cpu.SetXRegister(0x7F);
    cpu.Inx();
    CHECK(cpu.GetXRegister() == 0x80);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Negative flag set
}

TEST_CASE("Iny increments the Y register and updates the Zero and Negative flags") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.SetYRegister(0x09);
    cpu.Iny();
    CHECK(cpu.GetYRegister() == 0x0A);
    CHECK(cpu.StatusString() == "nvUbdIzc"); // both flags clear

    cpu.SetYRegister(0xFF);
    cpu.Iny();
    CHECK(cpu.GetYRegister() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc"); // wraps to 0x00, Zero flag set

    cpu.SetYRegister(0x7F);
    cpu.Iny();
    CHECK(cpu.GetYRegister() == 0x80);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Negative flag set
}

TEST_CASE("Dex decrements the X register and updates the Zero and Negative flags") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.SetXRegister(0x09);
    cpu.Dex();
    CHECK(cpu.GetXRegister() == 0x08);
    CHECK(cpu.StatusString() == "nvUbdIzc"); // both flags clear

    cpu.SetXRegister(0x01);
    cpu.Dex();
    CHECK(cpu.GetXRegister() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc"); // Zero flag set

    cpu.SetXRegister(0x00);
    cpu.Dex();
    CHECK(cpu.GetXRegister() == 0xFF);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // wraps to 0xFF, Negative flag set
}

TEST_CASE("Dey decrements the Y register and updates the Zero and Negative flags") {
    nes_test::TestBus bus;
    nes::Cpu cpu(bus);

    cpu.SetYRegister(0x09);
    cpu.Dey();
    CHECK(cpu.GetYRegister() == 0x08);
    CHECK(cpu.StatusString() == "nvUbdIzc"); // both flags clear

    cpu.SetYRegister(0x01);
    cpu.Dey();
    CHECK(cpu.GetYRegister() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZc"); // Zero flag set

    cpu.SetYRegister(0x00);
    cpu.Dey();
    CHECK(cpu.GetYRegister() == 0xFF);
    CHECK(cpu.StatusString() == "NvUbdIzc"); // wraps to 0xFF, Negative flag set
}