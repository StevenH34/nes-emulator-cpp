#include "doctest.h"

#include "../src/core/apu/Noise.h"

TEST_CASE("Noise starts disabled with a zero length counter and silent output") {
  nes::Noise noise;

  CHECK(noise.GetEnabled() == false);
  CHECK(noise.GetLengthCounter() == 0);
  CHECK(noise.Output() == 0);
}

TEST_CASE("SetEnabled toggles the enabled flag") {
  nes::Noise noise;

  noise.SetEnabled(true);
  CHECK(noise.GetEnabled() == true);

  noise.SetEnabled(false);
  CHECK(noise.GetEnabled() == false);
}

TEST_CASE("Disabling the channel immediately zeroes the length counter") {
  nes::Noise noise;
  noise.SetEnabled(true);
  noise.WriteLengthCounterRegister(0x08); // length index 1 -> 254
  REQUIRE(noise.GetLengthCounter() == 254);

  noise.SetEnabled(false);

  CHECK(noise.GetLengthCounter() == 0);
}

TEST_CASE("WriteLengthCounterRegister only loads the length counter while the channel is enabled") {
  nes::Noise noise;

  noise.WriteLengthCounterRegister(0x08); // channel still disabled
  CHECK(noise.GetLengthCounter() == 0);

  noise.SetEnabled(true);
  noise.WriteLengthCounterRegister(0x08);
  CHECK(noise.GetLengthCounter() == 254);
}

TEST_CASE("WriteLengthCounterRegister decodes the length index from the top 5 bits") {
  nes::Noise noise;
  noise.SetEnabled(true);

  noise.WriteLengthCounterRegister(0x00); // index 0 -> 10
  CHECK(noise.GetLengthCounter() == 10);

  noise.WriteLengthCounterRegister(0x18); // index 3 -> 2
  CHECK(noise.GetLengthCounter() == 2);
}

TEST_CASE("ClockLengthCounter decrements once per call until it reaches zero") {
  nes::Noise noise;
  noise.WriteControlRegister(0x00); // length halt off
  noise.SetEnabled(true);
  noise.WriteLengthCounterRegister(0x18); // index 3 -> 2

  noise.ClockLengthCounter();
  CHECK(noise.GetLengthCounter() == 1);

  noise.ClockLengthCounter();
  CHECK(noise.GetLengthCounter() == 0);

  noise.ClockLengthCounter(); // does not wrap past zero
  CHECK(noise.GetLengthCounter() == 0);
}

TEST_CASE("ClockLengthCounter does not decrement while length halt is set") {
  nes::Noise noise;
  noise.WriteControlRegister(0x20); // length halt bit
  noise.SetEnabled(true);
  noise.WriteLengthCounterRegister(0x18); // length counter = 2

  noise.ClockLengthCounter();

  CHECK(noise.GetLengthCounter() == 2);
}

TEST_CASE("Output returns the constant volume once the shift register's first bit clears") {
  nes::Noise noise;
  noise.SetEnabled(true);
  noise.WriteControlRegister(0x15); // length halt off, constant volume, volume 5
  noise.WriteLengthCounterRegister(0x00); // length counter = 10
  REQUIRE(noise.Output() == 0); // shift register still holds its power-up seed (1) -> muted

  noise.ClockTimer(); // advances the LFSR once, clearing bit 0

  CHECK(noise.Output() == 5);
}

TEST_CASE("Output is silent when the channel is disabled") {
  nes::Noise noise;
  noise.SetEnabled(true);
  noise.WriteControlRegister(0x15);
  noise.WriteLengthCounterRegister(0x00);
  noise.ClockTimer();
  REQUIRE(noise.Output() == 5); // sanity check: would otherwise be audible

  noise.SetEnabled(false);

  CHECK(noise.Output() == 0);
}

TEST_CASE("Output is silent once the length counter reaches zero") {
  nes::Noise noise;
  noise.WriteControlRegister(0x15); // length halt off, constant volume, volume 5
  noise.SetEnabled(true);
  noise.WriteLengthCounterRegister(0x18); // length index 3 -> 2
  noise.ClockTimer();
  REQUIRE(noise.Output() == 5);

  noise.ClockLengthCounter();
  noise.ClockLengthCounter();
  REQUIRE(noise.GetLengthCounter() == 0);

  CHECK(noise.Output() == 0);
}

TEST_CASE("ClockEnvelope resets the decay level to 15 on the first clock after being triggered") {
  nes::Noise noise;
  noise.SetEnabled(true);
  noise.WriteControlRegister(0x00); // constant volume off, envelope period 0
  noise.WriteLengthCounterRegister(0x00); // triggers envelope_start
  noise.ClockTimer(); // clears the power-up LFSR mute

  noise.ClockEnvelope();

  CHECK(noise.Output() == 15);
}

TEST_CASE("ClockEnvelope decays by one step per clock when the envelope period is zero") {
  nes::Noise noise;
  noise.SetEnabled(true);
  noise.WriteControlRegister(0x00);
  noise.WriteLengthCounterRegister(0x00);
  noise.ClockTimer();

  noise.ClockEnvelope(); // reload clock
  REQUIRE(noise.Output() == 15);

  noise.ClockEnvelope();
  CHECK(noise.Output() == 14);

  noise.ClockEnvelope();
  CHECK(noise.Output() == 13);
}

TEST_CASE("ClockEnvelope loops back to 15 once decay reaches 0 and length halt is set") {
  nes::Noise noise;
  noise.SetEnabled(true);
  noise.WriteControlRegister(0x20); // length halt on (doubles as envelope loop), period 0
  noise.WriteLengthCounterRegister(0x00);
  noise.ClockTimer();

  for (int i = 0; i < 16; ++i)
    noise.ClockEnvelope(); // 1 reload clock + 15 decay clocks -> reaches 0
  REQUIRE(noise.Output() == 0);

  noise.ClockEnvelope(); // divider hits 0 again with decay already at 0 -> loops
  CHECK(noise.Output() == 15);
}

TEST_CASE("ClockEnvelope stays at 0 once decayed when length halt is not set") {
  nes::Noise noise;
  noise.SetEnabled(true);
  noise.WriteControlRegister(0x00);
  noise.WriteLengthCounterRegister(0x00);
  noise.ClockTimer();

  for (int i = 0; i < 16; ++i)
    noise.ClockEnvelope();
  REQUIRE(noise.Output() == 0);

  noise.ClockEnvelope();

  CHECK(noise.Output() == 0);
}

TEST_CASE("The long-mode LFSR stays audible for 14 shifts from the power-up seed, then mutes on the 15th") {
  nes::Noise noise;
  noise.SetEnabled(true);
  noise.WriteControlRegister(0x15); // constant volume, volume 5
  noise.WriteLengthCounterRegister(0x00); // length counter = 10
  REQUIRE(noise.Output() == 0); // power-up seed is 1 -> bit 0 set -> muted

  // Mode defaults to long (M=0, feedback tap at bit 1); the timer period defaults to 0,
  // so every ClockTimer() call triggers exactly one LFSR shift. Starting from seed 1,
  // the lone set bit propagates cleanly down through the register for 14 shifts before
  // it reaches the tap and collides with bit 0 again on the 15th.
  for (int i = 0; i < 14; ++i) {
    noise.ClockTimer();
    CHECK(noise.Output() == 5);
  }

  noise.ClockTimer(); // 15th shift: the seed bit reaches the feedback tap and sets bit 0 again
  CHECK(noise.Output() == 0);
}

TEST_CASE("WriteModePeriod's long mode (M=0) still produces an audible first shift") {
  nes::Noise noise;
  noise.SetEnabled(true);
  noise.WriteControlRegister(0x15);
  noise.WriteLengthCounterRegister(0x00);

  noise.WriteModePeriod(0x00); // long mode, period index 0 -> period 4

  noise.ClockTimer(); // the first shift always fires immediately regardless of the period
  CHECK(noise.Output() == 5);
}

TEST_CASE("WriteModePeriod's short mode (M=1) still produces an audible first shift") {
  nes::Noise noise;
  noise.SetEnabled(true);
  noise.WriteControlRegister(0x15);
  noise.WriteLengthCounterRegister(0x00);

  noise.WriteModePeriod(0x80); // short mode, period index 0 -> period 4

  noise.ClockTimer();
  CHECK(noise.Output() == 5);
}
