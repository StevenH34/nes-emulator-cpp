#include "../doctest.h"

#include "../src/Bus.h"
#include "../src/Cpu.h"

TEST_CASE("Clc clears the Carry flag") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Sec();
    CHECK(cpu.StatusString() == "nvUbdIzC");

    cpu.Clc();
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("Sec sets the Carry flag") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    CHECK(cpu.StatusString() == "nvUbdIzc");

    cpu.Sec();
    CHECK(cpu.StatusString() == "nvUbdIzC");
}

TEST_CASE("Cli clears the Interrupt Disable flag") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    CHECK(cpu.StatusString() == "nvUbdIzc"); // I flag set on by default

    cpu.Cli();
    CHECK(cpu.StatusString() == "nvUbdizc");
}

TEST_CASE("Sei sets the Interrupt Disable flag") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Cli();
    CHECK(cpu.StatusString() == "nvUbdizc");

    cpu.Sei();
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("Cld clears the Decimal flag") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Sed();
    CHECK(cpu.StatusString() == "nvUbDIzc");

    cpu.Cld();
    CHECK(cpu.StatusString() == "nvUbdIzc");
}

TEST_CASE("Sed sets the Decimal flag") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    CHECK(cpu.StatusString() == "nvUbdIzc");

    cpu.Sed();
    CHECK(cpu.StatusString() == "nvUbDIzc");
}

TEST_CASE("Clv clears the Overflow flag") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.SetVFlag(true);
    CHECK(cpu.StatusString() == "nVUbdIzc");

    cpu.Clv();
    CHECK(cpu.StatusString() == "nvUbdIzc");
}