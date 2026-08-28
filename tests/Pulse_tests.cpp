#include "doctest.h"

#include "../src/core/apu/Pulse.h"

TEST_CASE("Pulse starts disabled with a zero length counter and silent output") {
    nes::Pulse pulse(0);

    CHECK(pulse.GetEnabled() == false);
    CHECK(pulse.GetLengthCounter() == 0);
    CHECK(pulse.Output() == 0);
}

TEST_CASE("SetEnabled toggles the enabled flag") {
    nes::Pulse pulse(0);

    pulse.SetEnabled(true);
    CHECK(pulse.GetEnabled() == true);

    pulse.SetEnabled(false);
    CHECK(pulse.GetEnabled() == false);
}

TEST_CASE("Disabling the channel immediately zeroes the length counter") {
    nes::Pulse pulse(0);
    pulse.SetEnabled(true);
    pulse.WriteTimerHigh(0x08); // length index 1 -> 254
    REQUIRE(pulse.GetLengthCounter() == 254);

    pulse.SetEnabled(false);

    CHECK(pulse.GetLengthCounter() == 0);
}

TEST_CASE("WriteTimerHigh only loads the length counter while the channel is enabled") {
    nes::Pulse pulse(0);

    pulse.WriteTimerHigh(0x08); // channel still disabled
    CHECK(pulse.GetLengthCounter() == 0);

    pulse.SetEnabled(true);
    pulse.WriteTimerHigh(0x08);
    CHECK(pulse.GetLengthCounter() == 254);
}

TEST_CASE("WriteTimerHigh decodes the length index from the top 5 bits") {
    nes::Pulse pulse(0);
    pulse.SetEnabled(true);

    pulse.WriteTimerHigh(0x00); // index 0 -> 10
    CHECK(pulse.GetLengthCounter() == 10);

    pulse.WriteTimerHigh(0x18); // index 3 -> 2
    CHECK(pulse.GetLengthCounter() == 2);
}

TEST_CASE("ClockLengthCounter decrements once per call until it reaches zero") {
    nes::Pulse pulse(0);
    pulse.SetEnabled(true);
    pulse.WriteTimerHigh(0x18); // index 3 -> 2

    pulse.ClockLengthCounter();
    CHECK(pulse.GetLengthCounter() == 1);

    pulse.ClockLengthCounter();
    CHECK(pulse.GetLengthCounter() == 0);

    pulse.ClockLengthCounter(); // does not wrap past zero
    CHECK(pulse.GetLengthCounter() == 0);
}

TEST_CASE("ClockLengthCounter does not decrement while length halt is set") {
    nes::Pulse pulse(0);
    pulse.WriteControl(0x20); // length halt bit
    pulse.SetEnabled(true);
    pulse.WriteTimerHigh(0x18); // length counter = 2

    pulse.ClockLengthCounter();

    CHECK(pulse.GetLengthCounter() == 2);
}

TEST_CASE("Output returns the constant volume when nothing mutes the channel") {
    nes::Pulse pulse(0);
    pulse.SetEnabled(true);
    pulse.WriteControl(0xD5); // duty 75% (step 0 is high), constant volume, volume 5
    pulse.WriteTimerLow(0x08);
    pulse.WriteTimerHigh(0x00); // timer period 8, length counter loaded

    CHECK(pulse.Output() == 5);
}

TEST_CASE("Output is silent when the channel is disabled") {
    nes::Pulse pulse(0);
    pulse.SetEnabled(true);
    pulse.WriteControl(0xD5);
    pulse.WriteTimerLow(0x08);
    pulse.WriteTimerHigh(0x00);
    REQUIRE(pulse.Output() == 5); // sanity check: would otherwise be audible

    pulse.SetEnabled(false);

    CHECK(pulse.Output() == 0);
}

TEST_CASE("Output is silent once the length counter reaches zero") {
    nes::Pulse pulse(0);
    pulse.SetEnabled(true);
    pulse.WriteControl(0xD5);
    pulse.WriteTimerLow(0x08);
    pulse.WriteTimerHigh(0x18); // length index 3 -> 2
    REQUIRE(pulse.Output() == 5);

    pulse.ClockLengthCounter();
    pulse.ClockLengthCounter();
    REQUIRE(pulse.GetLengthCounter() == 0);

    CHECK(pulse.Output() == 0);
}

TEST_CASE("Output is silent when the selected duty step is low") {
    nes::Pulse pulse(0);
    pulse.SetEnabled(true);
    pulse.WriteControl(0x15); // duty 12.5% (step 0 is low), constant volume, volume 5
    pulse.WriteTimerLow(0x08);
    pulse.WriteTimerHigh(0x00);

    CHECK(pulse.Output() == 0);
}

TEST_CASE("Output is silent when the timer period is below the 8-cycle floor") {
    nes::Pulse pulse(0);
    pulse.SetEnabled(true);
    pulse.WriteControl(0xD5); // duty 75% (step 0 is high)
    pulse.WriteTimerLow(0x07); // timer period 7, below the floor
    pulse.WriteTimerHigh(0x00);

    CHECK(pulse.Output() == 0);
}

TEST_CASE("Output mutes when the computed sweep target exceeds the 11-bit range, "
          "even though the sweep unit was never enabled") {
    nes::Pulse pulse(0);
    pulse.SetEnabled(true);
    pulse.WriteControl(0xD5); // duty 75%, constant volume 5
    pulse.WriteTimerLow(0x00);
    pulse.WriteTimerHigh(0x04); // timer period 0x400 (1024); sweep registers untouched

    CHECK(pulse.Output() == 0);
}

TEST_CASE("ClockEnvelope resets the decay level to 15 on the first clock after being triggered") {
    nes::Pulse pulse(0);
    pulse.SetEnabled(true);
    pulse.WriteControl(0xC0); // duty 75% (step 0 is high), constant volume off, envelope period 0
    pulse.WriteTimerLow(0x08);
    pulse.WriteTimerHigh(0x00); // triggers envelope_start

    pulse.ClockEnvelope();

    CHECK(pulse.Output() == 15);
}

TEST_CASE("ClockEnvelope decays by one step per clock when the envelope period is zero") {
    nes::Pulse pulse(0);
    pulse.SetEnabled(true);
    pulse.WriteControl(0xC0); // duty 75%, constant volume off, envelope period 0
    pulse.WriteTimerLow(0x08);
    pulse.WriteTimerHigh(0x00);

    pulse.ClockEnvelope(); // reload clock
    REQUIRE(pulse.Output() == 15);

    pulse.ClockEnvelope();
    CHECK(pulse.Output() == 14);

    pulse.ClockEnvelope();
    CHECK(pulse.Output() == 13);
}

TEST_CASE("ClockEnvelope loops back to 15 once decay reaches 0 and the loop flag is set") {
    nes::Pulse pulse(0);
    pulse.SetEnabled(true);
    pulse.WriteControl(0xE0); // duty 75%, length halt / envelope loop set, envelope period 0
    pulse.WriteTimerLow(0x08);
    pulse.WriteTimerHigh(0x00);

    for (int i = 0; i < 16; ++i) pulse.ClockEnvelope(); // 1 reload clock + 15 decay clocks -> reaches 0
    REQUIRE(pulse.Output() == 0);

    pulse.ClockEnvelope(); // divider hits 0 again with decay already at 0 -> loops
    CHECK(pulse.Output() == 15);
}

TEST_CASE("ClockEnvelope stays at 0 once decayed when the loop flag is not set") {
    nes::Pulse pulse(0);
    pulse.SetEnabled(true);
    pulse.WriteControl(0xC0); // duty 75%, no loop flag, envelope period 0
    pulse.WriteTimerLow(0x08);
    pulse.WriteTimerHigh(0x00);

    for (int i = 0; i < 16; ++i) pulse.ClockEnvelope();
    REQUIRE(pulse.Output() == 0);

    pulse.ClockEnvelope();

    CHECK(pulse.Output() == 0);
}

TEST_CASE("Pulse 1's sweep negation subtracts one extra (one's complement), unlike Pulse 2 (two's complement)") {
    nes::Pulse pulse1(0);
    pulse1.SetEnabled(true);
    pulse1.WriteControl(0xD5); // duty 75% (step 0 is high), constant volume 5
    pulse1.WriteTimerLow(0x08);
    pulse1.WriteTimerHigh(0x00); // timer period 8
    pulse1.WriteSweep(0x08); // negate enabled, shift 0 -> change equals the timer period itself

    nes::Pulse pulse2(1);
    pulse2.SetEnabled(true);
    pulse2.WriteControl(0xD5);
    pulse2.WriteTimerLow(0x08);
    pulse2.WriteTimerHigh(0x00);
    pulse2.WriteSweep(0x08);

    // Pulse 1: target = 8 - 8 - 1 underflows to a huge UInt16, which mutes the channel.
    CHECK(pulse1.Output() == 0);
    // Pulse 2: target = 8 - 8 = 0, which is in range and stays audible.
    CHECK(pulse2.Output() == 5);
}

TEST_CASE("ClockSweep raises the timer period toward the target when due, enabled, and shift is nonzero") {
    nes::Pulse pulse(0);
    pulse.WriteTimerLow(0x08);
    pulse.WriteTimerHigh(0x00); // timer period 8
    pulse.WriteSweep(0x81); // enabled, shift 1, positive, period 0 -> target = 8 + (8 >> 1) = 12

    pulse.ClockSweep();

    CHECK(pulse.GetTimerPeriod() == 12);
}

TEST_CASE("ClockSweep leaves the timer period unchanged when the shift is zero") {
    nes::Pulse pulse(0);
    pulse.WriteTimerLow(0x08);
    pulse.WriteTimerHigh(0x00);
    pulse.WriteSweep(0x80); // enabled, shift 0

    pulse.ClockSweep();

    CHECK(pulse.GetTimerPeriod() == 8);
}

TEST_CASE("ClockSweep leaves the timer period unchanged when the sweep unit is not enabled") {
    nes::Pulse pulse(0);
    pulse.WriteTimerLow(0x08);
    pulse.WriteTimerHigh(0x00);
    pulse.WriteSweep(0x01); // shift 1, but not enabled

    pulse.ClockSweep();

    CHECK(pulse.GetTimerPeriod() == 8);
}

TEST_CASE("ClockSweep only applies the target once per full divider period") {
    nes::Pulse pulse(0);
    pulse.WriteTimerLow(0x08);
    pulse.WriteTimerHigh(0x00); // timer period 8
    pulse.WriteSweep(0xA1); // enabled, shift 1, period 2 -> divider counts 2, 1, 0 between updates

    pulse.ClockSweep(); // divider starts at 0 -> applies immediately: target = 8 + 4 = 12
    CHECK(pulse.GetTimerPeriod() == 12);

    pulse.ClockSweep(); // divider reloaded to 2, counts down: no update
    CHECK(pulse.GetTimerPeriod() == 12);

    pulse.ClockSweep(); // divider at 1: no update
    CHECK(pulse.GetTimerPeriod() == 12);

    pulse.ClockSweep(); // divider reaches 0: target = 12 + (12 >> 1) = 18
    CHECK(pulse.GetTimerPeriod() == 18);
}
