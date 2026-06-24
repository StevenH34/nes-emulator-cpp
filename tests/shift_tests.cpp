#include "../doctest.h"

#include "../src/Bus.h"
#include "../src/Cpu.h"

TEST_CASE("AslAccumulator shifts bits left and clears Carry, Zero, and Negative") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x01); // 0000 0001

    cpu.AslAccumulator();

    CHECK(cpu.GetAccumulator() == 0x02); // 0000 0010
    CHECK(cpu.StatusString() == "nvUbdIzc"); // Carry clear, Zero clear, Negative clear
}

TEST_CASE("AslAccumulator sets Carry when bit 7 is shifted out") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x81); // 1000 0001

    cpu.AslAccumulator();

    CHECK(cpu.GetAccumulator() == 0x02); // 0000 0010, bit 7 dropped
    CHECK(cpu.StatusString() == "nvUbdIzC"); // Carry set, Zero clear, Negative clear
}

TEST_CASE("AslAccumulator sets Zero when the result is 0x00") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x80); // 1000 0000

    cpu.AslAccumulator();

    CHECK(cpu.GetAccumulator() == 0x00);
    CHECK(cpu.StatusString() == "nvUbdIZC"); // Carry set, Zero set, Negative clear
}

TEST_CASE("AslAccumulator sets Negative when bit 7 of the result is set") {
    nes::Bus bus;
    nes::Cpu cpu(bus);

    cpu.Lda(0x40); // 0100 0000

    cpu.AslAccumulator();

    CHECK(cpu.GetAccumulator() == 0x80); // 1000 0000
    CHECK(cpu.StatusString() == "NvUbdIzc"); // Carry clear, Zero clear, Negative set
}