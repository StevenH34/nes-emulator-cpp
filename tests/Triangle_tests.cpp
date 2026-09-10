#include "doctest.h"

#include "../src/core/apu/Triangle.h"

TEST_CASE("Triangle starts disabled with a zero length counter and silent output") {
  nes::Triangle triangle;

  CHECK(triangle.GetEnabled() == false);
  CHECK(triangle.GetLengthCounter() == 0);
  CHECK(triangle.Output() == 0);
}

TEST_CASE("SetEnabled toggles the enabled flag") {
  nes::Triangle triangle;

  triangle.SetEnabled(true);
  CHECK(triangle.GetEnabled() == true);

  triangle.SetEnabled(false);
  CHECK(triangle.GetEnabled() == false);
}

TEST_CASE("Disabling the channel immediately zeroes the length counter") {
  nes::Triangle triangle;
  triangle.SetEnabled(true);
  triangle.WriteTimerHigh(0x08); // length index 1 -> 254
  REQUIRE(triangle.GetLengthCounter() == 254);

  triangle.SetEnabled(false);

  CHECK(triangle.GetLengthCounter() == 0);
}

TEST_CASE("WriteTimerHigh only loads the length counter while the channel is enabled") {
  nes::Triangle triangle;

  triangle.WriteTimerHigh(0x08); // channel still disabled
  CHECK(triangle.GetLengthCounter() == 0);

  triangle.SetEnabled(true);
  triangle.WriteTimerHigh(0x08);
  CHECK(triangle.GetLengthCounter() == 254);
}

TEST_CASE("WriteTimerHigh decodes the length index from the top 5 bits") {
  nes::Triangle triangle;
  triangle.SetEnabled(true);

  triangle.WriteTimerHigh(0x00); // index 0 -> 10
  CHECK(triangle.GetLengthCounter() == 10);

  triangle.WriteTimerHigh(0x18); // index 3 -> 2
  CHECK(triangle.GetLengthCounter() == 2);
}

TEST_CASE("ClockLengthCounter decrements once per call until it reaches zero") {
  nes::Triangle triangle;
  triangle.WriteLinearCounter(0x00); // control flag off
  triangle.SetEnabled(true);
  triangle.WriteTimerHigh(0x18); // index 3 -> 2

  triangle.ClockLengthCounter();
  CHECK(triangle.GetLengthCounter() == 1);

  triangle.ClockLengthCounter();
  CHECK(triangle.GetLengthCounter() == 0);

  triangle.ClockLengthCounter(); // does not wrap past zero
  CHECK(triangle.GetLengthCounter() == 0);
}

TEST_CASE("ClockLengthCounter does not decrement while the control flag is set") {
  nes::Triangle triangle;
  triangle.WriteLinearCounter(0x80); // control flag on (doubles as length halt)
  triangle.SetEnabled(true);
  triangle.WriteTimerHigh(0x18); // length counter = 2

  triangle.ClockLengthCounter();

  CHECK(triangle.GetLengthCounter() == 2);
}

TEST_CASE("Output returns the sequence value when nothing mutes the channel") {
  nes::Triangle triangle;
  triangle.SetEnabled(true);
  triangle.WriteLinearCounter(0x7F); // control flag off, reload 127
  triangle.WriteTimerLow(0x02);
  triangle.WriteTimerHigh(0x00); // timer period 2, length counter loaded, linear reload flag set
  triangle.ClockLinearCounter(); // linear counter loaded to 127

  CHECK(triangle.Output() == 15); // sequence position starts at 0 -> SEQUENCE_TABLE[0]
}

TEST_CASE("Output is silent when the channel is disabled") {
  nes::Triangle triangle;
  triangle.SetEnabled(true);
  triangle.WriteLinearCounter(0x7F);
  triangle.WriteTimerLow(0x02);
  triangle.WriteTimerHigh(0x00);
  triangle.ClockLinearCounter();
  REQUIRE(triangle.Output() == 15); // sanity check: would otherwise be audible

  triangle.SetEnabled(false);

  CHECK(triangle.Output() == 0);
}

TEST_CASE("Output is silent once the length counter reaches zero") {
  nes::Triangle triangle;
  triangle.SetEnabled(true);
  triangle.WriteLinearCounter(0x7F);
  triangle.WriteTimerLow(0x02);
  triangle.WriteTimerHigh(0x18); // length index 3 -> 2
  triangle.ClockLinearCounter();
  REQUIRE(triangle.Output() == 15);

  triangle.ClockLengthCounter();
  triangle.ClockLengthCounter();
  REQUIRE(triangle.GetLengthCounter() == 0);

  CHECK(triangle.Output() == 0);
}

TEST_CASE("Output is silent when the linear counter is zero") {
  nes::Triangle triangle;
  triangle.SetEnabled(true);
  triangle.WriteLinearCounter(0x7F);
  triangle.WriteTimerLow(0x02);
  triangle.WriteTimerHigh(0x00); // length counter loaded, but linear counter never clocked

  CHECK(triangle.Output() == 0);
}

TEST_CASE("Output is silent when the timer period is below the 2-cycle floor") {
  nes::Triangle triangle;
  triangle.SetEnabled(true);
  triangle.WriteLinearCounter(0x7F);
  triangle.WriteTimerLow(0x01); // timer period 1, below the floor
  triangle.WriteTimerHigh(0x00);
  triangle.ClockLinearCounter();

  CHECK(triangle.Output() == 0);
}

TEST_CASE("ClockTimer advances the sequence position once the timer period elapses") {
  nes::Triangle triangle;
  triangle.SetEnabled(true);
  triangle.WriteLinearCounter(0x7F);
  triangle.WriteTimerLow(0x02);
  triangle.WriteTimerHigh(0x00);
  triangle.ClockLinearCounter();
  REQUIRE(triangle.Output() == 15); // sequence position 0

  triangle.ClockTimer(); // timer starts at 0 -> reloads and advances immediately

  CHECK(triangle.Output() == 14); // sequence position 1
}

TEST_CASE("ClockTimer does not advance the sequence position while the length counter is zero") {
  nes::Triangle triangle;
  triangle.WriteLinearCounter(0x7F);
  triangle.WriteTimerLow(0x02);
  triangle.WriteTimerHigh(0x00); // channel disabled -> length counter stays 0
  triangle.SetEnabled(true);
  triangle.ClockLinearCounter(); // linear counter loaded, but length counter is still 0
  REQUIRE(triangle.GetLengthCounter() == 0);
  REQUIRE(triangle.Output() == 0); // muted by the zero length counter, confirms setup

  triangle.ClockTimer(); // must not advance the sequence position while length counter is 0

  triangle.WriteTimerHigh(0x08); // now give it a real length counter; does not reset sequence position
  CHECK(triangle.Output() == 15); // still sequence position 0, proving ClockTimer did not advance it above
}

TEST_CASE("ClockTimer does not advance the sequence position while the linear counter is zero") {
  nes::Triangle triangle;
  triangle.SetEnabled(true);
  triangle.WriteLinearCounter(0x7F);
  triangle.WriteTimerLow(0x02);
  triangle.WriteTimerHigh(0x18); // length counter loaded, but linear counter not clocked yet
  REQUIRE(triangle.Output() == 0); // muted by the zero linear counter, confirms setup

  triangle.ClockTimer(); // must not advance the sequence position while linear counter is 0

  triangle.ClockLinearCounter(); // now load the linear counter
  CHECK(triangle.Output() == 15); // still sequence position 0, proving ClockTimer did not advance it above
}

TEST_CASE("ClockLinearCounter reloads then decrements toward silence when the control flag is not set") {
  nes::Triangle triangle;
  triangle.SetEnabled(true);
  triangle.WriteLinearCounter(0x02); // control flag off, reload 2
  triangle.WriteTimerLow(0x02);
  triangle.WriteTimerHigh(0x00);
  REQUIRE(triangle.Output() == 0); // not yet reloaded

  triangle.ClockLinearCounter(); // reloads to 2, clears the reload flag
  CHECK(triangle.Output() == 15);

  triangle.ClockLinearCounter(); // decrements: 2 -> 1
  CHECK(triangle.Output() == 15);

  triangle.ClockLinearCounter(); // decrements: 1 -> 0
  CHECK(triangle.Output() == 0);
}

TEST_CASE("ClockLinearCounter keeps reloading (sustains) while the control flag is set") {
  nes::Triangle triangle;
  triangle.SetEnabled(true);
  triangle.WriteLinearCounter(0x82); // control flag on, reload 2
  triangle.WriteTimerLow(0x02);
  triangle.WriteTimerHigh(0x00);

  triangle.ClockLinearCounter(); // reloads to 2; reload flag stays set since control flag is on
  CHECK(triangle.Output() == 15);

  triangle.ClockLinearCounter(); // reloads to 2 again instead of decrementing
  CHECK(triangle.Output() == 15);

  triangle.ClockLinearCounter();
  CHECK(triangle.Output() == 15); // still sustained, would have hit 0 by now if it were decrementing
}

TEST_CASE("WriteTimerHigh retriggers the linear counter reload even without rewriting $4008") {
  nes::Triangle triangle;
  triangle.SetEnabled(true);
  triangle.WriteLinearCounter(0x05); // control flag off, reload 5
  triangle.WriteTimerLow(0x02);
  triangle.WriteTimerHigh(0x00);
  triangle.ClockLinearCounter(); // linear counter = 5
  triangle.ClockLinearCounter(); // decrements to 4
  REQUIRE(triangle.Output() == 15);

  triangle.WriteTimerHigh(0x00); // retrigger: sets the reload flag again, $4008 untouched
  triangle.ClockLinearCounter(); // should reload back to 5, not continue decrementing from 4

  for (int i = 0; i < 4; ++i)
    triangle.ClockLinearCounter(); // 5 -> 4 -> 3 -> 2 -> 1
  REQUIRE(triangle.Output() == 15); // still audible at 1

  triangle.ClockLinearCounter(); // 1 -> 0
  CHECK(triangle.Output() == 0); // took exactly 5 decrements after the retrigger, confirming the reload
}
