#include "doctest.h"

#include "../src/core/Controller.h"
#include "TestBus.h"

TEST_CASE("Controller reads all zero bits when no buttons are pressed") {
    nes::Controller controller;

    controller.Write(1);
    controller.Write(0);

    for (int i = 0; i < 8; ++i) {
        CHECK(controller.Read() == 0);
    }
}

TEST_CASE("Controller reports a single pressed button in the correct bit position") {
    // Shift-out order is A, B, Select, Start, Up, Down, Left, Right.
    nes::Controller controller;
    controller.Press(nes::Controller::BUTTON_START);

    controller.Write(1);
    controller.Write(0);

    const std::uint8_t expected[8] = {0, 0, 0, 1, 0, 0, 0, 0};
    for (const std::uint8_t bit : expected) {
        CHECK(controller.Read() == bit);
    }
}

TEST_CASE("Controller reports multiple pressed buttons in the correct bit positions") {
    nes::Controller controller;
    controller.Press(nes::Controller::BUTTON_A);
    controller.Press(nes::Controller::BUTTON_START);
    controller.Press(nes::Controller::BUTTON_RIGHT);

    controller.Write(1);
    controller.Write(0);

    const std::uint8_t expected[8] = {1, 0, 0, 1, 0, 0, 0, 1};
    for (const std::uint8_t bit : expected) {
        CHECK(controller.Read() == bit);
    }
}

TEST_CASE("Controller reads past the 8th bit return 0 until the next strobe") {
    nes::Controller controller;
    controller.Press(nes::Controller::BUTTON_A);

    controller.Write(1);
    controller.Write(0);

    for (int i = 0; i < 8; ++i) {
        controller.Read();
    }

    CHECK(controller.Read() == 0);
    CHECK(controller.Read() == 0);
}

TEST_CASE("Controller Release clears a pressed button") {
    nes::Controller controller;
    controller.Press(nes::Controller::BUTTON_A);
    controller.Release(nes::Controller::BUTTON_A);

    controller.Write(1);
    controller.Write(0);

    CHECK(controller.Read() == 0);
}

TEST_CASE("Controller Release only affects the targeted button") {
    nes::Controller controller;
    controller.Press(nes::Controller::BUTTON_A);
    controller.Press(nes::Controller::BUTTON_B);
    controller.Release(nes::Controller::BUTTON_A);

    controller.Write(1);
    controller.Write(0);

    CHECK(controller.Read() == 0); // A
    CHECK(controller.Read() == 1); // B
}

TEST_CASE("Controller keeps returning the live A button state while strobe stays high") {
    nes::Controller controller;

    controller.Write(1);
    CHECK(controller.Read() == 0);

    controller.Press(nes::Controller::BUTTON_A);
    CHECK(controller.Read() == 1);

    controller.Release(nes::Controller::BUTTON_A);
    CHECK(controller.Read() == 0);
}

TEST_CASE("Controller can be strobed a second time to latch newly pressed buttons") {
    nes::Controller controller;
    controller.Press(nes::Controller::BUTTON_A);

    controller.Write(1);
    controller.Write(0);
    for (int i = 0; i < 8; ++i) {
        controller.Read();
    }

    controller.Press(nes::Controller::BUTTON_B);
    controller.Write(1);
    controller.Write(0);

    CHECK(controller.Read() == 1); // A
    CHECK(controller.Read() == 1); // B
}

TEST_CASE("Controller buttons pressed after latching do not affect the current read-out") {
    nes::Controller controller;
    controller.Press(nes::Controller::BUTTON_A);

    controller.Write(1);
    controller.Write(0);

    // Pressing more buttons after the strobe goes low must not change the
    // already-latched shift register.
    controller.Press(nes::Controller::BUTTON_B);

    CHECK(controller.Read() == 1); // A, latched before BUTTON_B was pressed
    CHECK(controller.Read() == 0); // B was not part of the latched snapshot
}

TEST_CASE("Controller Write with the strobe bit already low leaves the shift register at its initial state") {
    nes::Controller controller;
    controller.Press(nes::Controller::BUTTON_A);

    controller.Write(0); // strobe never went high, so nothing was latched

    CHECK(controller.Read() == 0);
}

TEST_CASE("Bus strobes and reads controller 1 through $4016") {
    nes_test::TestBus bus;
    bus.GetController1().Press(nes::Controller::BUTTON_START);

    bus.WriteCpu(0x4016, 1);
    bus.WriteCpu(0x4016, 0);

    const std::uint8_t expected[8] = {0, 0, 0, 1, 0, 0, 0, 0};
    for (const std::uint8_t bit : expected) {
        CHECK(bus.ReadCpu(0x4016) == bit);
    }
}

TEST_CASE("Bus write to $4016 strobes both controllers") {
    nes_test::TestBus bus;
    bus.GetController1().Press(nes::Controller::BUTTON_A);
    bus.GetController2().Press(nes::Controller::BUTTON_B);

    bus.WriteCpu(0x4016, 1);
    bus.WriteCpu(0x4016, 0);

    CHECK(bus.ReadCpu(0x4016) == 1); // Controller 1's A
    CHECK(bus.ReadCpu(0x4017) == 0); // Controller 2's A (not pressed)
    CHECK(bus.ReadCpu(0x4017) == 1); // Controller 2's B
}

TEST_CASE("Bus reads controller 2 independently of controller 1 through $4017") {
    nes_test::TestBus bus;
    bus.GetController2().Press(nes::Controller::BUTTON_SELECT);

    bus.WriteCpu(0x4016, 1);
    bus.WriteCpu(0x4016, 0);

    const std::uint8_t expected[8] = {0, 0, 1, 0, 0, 0, 0, 0};
    for (const std::uint8_t bit : expected) {
        CHECK(bus.ReadCpu(0x4017) == bit);
    }
}