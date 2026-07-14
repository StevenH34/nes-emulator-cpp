#include "doctest.h"

#include "../src/Bus.h"
#include "TestBus.h"

TEST_CASE("Bus RAM reads and writes are mirrored across the 2 KB window") {
    nes_test::TestBus bus;

    bus.WriteCpu(0x07FF, 0xAB);

    CHECK(bus.ReadCpu(0x07FF) == 0xAB);
    CHECK(bus.ReadCpu(0x0FFF) == 0xAB);
    CHECK(bus.ReadCpu(0x17FF) == 0xAB);
    CHECK(bus.ReadCpu(0x1FFF) == 0xAB);
}

TEST_CASE("Bus RAM read and write helpers access the mirrored backing store") {
    nes_test::TestBus bus;

    bus.WriteRam(0x0000, 0x11);
    bus.WriteRam(0x0800, 0x22);

    CHECK(bus.ReadRam(0x0000) == 0x22);
    CHECK(bus.ReadRam(0x0800) == 0x22);
    CHECK(bus.ReadRam(0x1000) == 0x22);
    CHECK(bus.ReadRam(0x1800) == 0x22);
}

TEST_CASE("Bus RAM starts zero-initialized") {
    nes_test::TestBus bus;

    CHECK(bus.ReadCpu(0x0000) == 0x00);
    CHECK(bus.ReadCpu(0x07FF) == 0x00);
}

TEST_CASE("Bus writes do not bleed into unrelated, non-mirrored addresses") {
    nes_test::TestBus bus;

    bus.WriteCpu(0x0010, 0x99);

    CHECK(bus.ReadCpu(0x0010) == 0x99);
    CHECK(bus.ReadCpu(0x0011) == 0x00);
    CHECK(bus.ReadCpu(0x0800) == 0x00); // mirror of 0x0000, not 0x0010
}

TEST_CASE("Bus returns zero for addresses outside the RAM mirror window") {
    nes_test::TestBus bus;

    CHECK(bus.ReadCpu(0x2000) == 0x00);
    CHECK(bus.ReadCpu(0x8000) == 0x00);
    CHECK(bus.ReadCpu(0xFFFF) == 0x00);
}

TEST_CASE("Bus ignores writes to addresses outside the RAM mirror window") {
    nes_test::TestBus bus;

    bus.WriteCpu(0x2000, 0xAB);

    CHECK(bus.ReadCpu(0x2000) == 0x00);
    // A naive address-mask without a range check would alias 0x2000 onto 0x0000
    CHECK(bus.ReadCpu(0x0000) == 0x00);
}


