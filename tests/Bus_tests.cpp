#include "../doctest.h"

#include "../Bus.h"

TEST_CASE("Bus RAM reads and writes are mirrored across the 2 KB window") {
    nes::Bus bus;

    bus.WriteCpu(0x07FF, 0xAB);

    CHECK(bus.ReadCpu(0x07FF) == 0xAB);
    CHECK(bus.ReadCpu(0x0FFF) == 0xAB);
    CHECK(bus.ReadCpu(0x17FF) == 0xAB);
    CHECK(bus.ReadCpu(0x1FFF) == 0xAB);
}

TEST_CASE("Bus RAM read and write helpers access the mirrored backing store") {
    nes::Bus bus;

    bus.WriteRam(0x0000, 0x11);
    bus.WriteRam(0x0800, 0x22);

    CHECK(bus.ReadRam(0x0000) == 0x22);
    CHECK(bus.ReadRam(0x0800) == 0x22);
    CHECK(bus.ReadRam(0x1000) == 0x22);
}


