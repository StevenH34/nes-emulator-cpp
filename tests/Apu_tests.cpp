#include "doctest.h"

#include "../src/core/apu/Apu.h"
#include "../src/core/save_state/StateReader.h"
#include "../src/core/save_state/StateWriter.h"

#include <cstdint>
#include <vector>

TEST_CASE("Step advances the cycle counter by exactly the number of CPU cycles passed in") {
  nes::Apu apu;

  apu.Step(100);
  CHECK(apu.GetCycle() == 100);

  apu.Step(50);
  CHECK(apu.GetCycle() == 150);
}

TEST_CASE("WriteRegisters routes $4003 to Pulse 1 and $4007 to Pulse 2 independently") {
  nes::Apu apu;
  apu.GetPulse1().SetEnabled(true);
  apu.GetPulse2().SetEnabled(true);

  apu.WriteRegisters(0x4003, 0x08); // Pulse 1 timer-high: length index 1 -> 254
  CHECK(apu.GetPulse1().GetLengthCounter() == 254);
  CHECK(apu.GetPulse2().GetLengthCounter() == 0);

  apu.WriteRegisters(0x4007, 0x18); // Pulse 2 timer-high: length index 3 -> 2
  CHECK(apu.GetPulse1().GetLengthCounter() == 254); // unchanged
  CHECK(apu.GetPulse2().GetLengthCounter() == 2);
}

TEST_CASE("WriteRegisters routes $400B to Triangle and $400F to Noise") {
  nes::Apu apu;
  apu.GetTriangle().SetEnabled(true);
  apu.GetNoise().SetEnabled(true);

  apu.WriteRegisters(0x400B, 0x08); // Triangle timer-high: length index 1 -> 254
  CHECK(apu.GetTriangle().GetLengthCounter() == 254);
  CHECK(apu.GetNoise().GetLengthCounter() == 0);

  apu.WriteRegisters(0x400F, 0x18); // Noise length counter: length index 3 -> 2
  CHECK(apu.GetTriangle().GetLengthCounter() == 254); // unchanged
  CHECK(apu.GetNoise().GetLengthCounter() == 2);
}

TEST_CASE("WriteRegisters routes Pulse 1's control and timer registers correctly") {
  nes::Apu apu;
  apu.GetPulse1().SetEnabled(true);

  apu.WriteRegisters(0x4000, 0xD5); // duty 75% (step 0 is high), constant volume 5
  apu.WriteRegisters(0x4002, 0x08); // timer low
  apu.WriteRegisters(0x4003, 0x00); // timer high -> period 8, length counter 10

  CHECK(apu.GetPulse1().Output() == 5);
}

TEST_CASE("WriteRegisters ignores unused and DMC register addresses") {
  nes::Apu apu;
  apu.GetPulse1().SetEnabled(true);
  apu.WriteRegisters(0x4003, 0x08);
  REQUIRE(apu.GetPulse1().GetLengthCounter() == 254);

  apu.WriteRegisters(0x4009, 0xFF); // unused
  apu.WriteRegisters(0x400D, 0xFF); // unused
  apu.WriteRegisters(0x4010, 0xFF); // DMC, not implemented
  apu.WriteRegisters(0x4013, 0xFF); // DMC, not implemented

  CHECK(apu.GetPulse1().GetLengthCounter() == 254); // unaffected
}

TEST_CASE("ReadStatus only reflects a channel once its length counter is loaded, not just enabled") {
  nes::Apu apu;

  apu.WriteStatus(0x01); // enable Pulse 1 only
  CHECK(apu.ReadStatus() == 0x00); // length counter is still 0, so no bit is set

  apu.WriteRegisters(0x4003, 0x08); // Pulse 1 timer-high: loads the length counter
  CHECK(apu.ReadStatus() == 0x01); // now Pulse 1's bit shows up
}

TEST_CASE("ReadStatus reports each channel independently") {
  nes::Apu apu;
  apu.WriteStatus(0x0F); // enable all four channels
  apu.WriteRegisters(0x4003, 0x08); // Pulse 1 length
  apu.WriteRegisters(0x4007, 0x08); // Pulse 2 length
  apu.WriteRegisters(0x400B, 0x08); // Triangle length
  apu.WriteRegisters(0x400F, 0x08); // Noise length

  CHECK(apu.ReadStatus() == 0x0F); // all four bits set
}

TEST_CASE("WriteStatus disabling a channel zeroes its length counter, clearing its ReadStatus bit") {
  nes::Apu apu;
  apu.WriteStatus(0x01);
  apu.WriteRegisters(0x4003, 0x08);
  REQUIRE(apu.ReadStatus() == 0x01);

  apu.WriteStatus(0x00); // disable Pulse 1
  CHECK(apu.ReadStatus() == 0x00);
}

TEST_CASE("WriteFrameCounter sets the mode and resets the frame cycle") {
  nes::Apu apu;
  apu.Step(10000); // advance frame_cycle partway through a sequence

  apu.WriteFrameCounter(0x00); // mode 0
  CHECK(apu.GetFrameMode() == 0);
  CHECK(apu.GetFrameCycle() == 0);

  apu.Step(10000);

  apu.WriteFrameCounter(0x80); // mode 1
  CHECK(apu.GetFrameMode() == 1);
  CHECK(apu.GetFrameCycle() == 0);
}

TEST_CASE("WriteFrameCounter in mode 1 immediately clocks quarter and half frame events") {
  nes::Apu apu;
  apu.GetPulse1().SetEnabled(true);
  apu.GetPulse1().WriteControl(0x00); // length halt off
  apu.GetPulse1().WriteTimerHigh(0x18); // length counter = 2
  REQUIRE(apu.GetPulse1().GetLengthCounter() == 2);

  apu.WriteFrameCounter(0x80); // mode 1: immediately clocks quarter + half frame

  // ClockHalfFrame clocks the length counter, so it should decrement immediately,
  // without waiting for Step() to reach a frame-step boundary.
  CHECK(apu.GetPulse1().GetLengthCounter() == 1);
}

TEST_CASE("TakeSample mixes all four channel outputs using the documented coefficients") {
  nes::Apu apu;

  // Configure each channel to produce a known, nonzero Output().
  apu.GetPulse1().SetEnabled(true);
  apu.GetPulse1().WriteControl(0xD5); // duty 75%, constant volume, volume 5
  apu.GetPulse1().WriteTimerLow(0x08);
  apu.GetPulse1().WriteTimerHigh(0x00); // period 8, length counter 10

  apu.GetPulse2().SetEnabled(true);
  apu.GetPulse2().WriteControl(0xD3); // duty 75%, constant volume, volume 3
  apu.GetPulse2().WriteTimerLow(0x08);
  apu.GetPulse2().WriteTimerHigh(0x00);

  apu.GetTriangle().SetEnabled(true);
  apu.GetTriangle().WriteLinearCounter(0x7F); // control off, reload 127
  apu.GetTriangle().WriteTimerLow(0x02);
  apu.GetTriangle().WriteTimerHigh(0x00); // period 2, length counter 10
  apu.GetTriangle().ClockLinearCounter(); // loads the linear counter to 127

  apu.GetNoise().SetEnabled(true);
  apu.GetNoise().WriteControlRegister(0x15); // constant volume, volume 5
  apu.GetNoise().WriteLengthCounterRegister(0x00); // length counter 10
  apu.GetNoise().ClockTimer(); // clears the power-up LFSR mute

  const auto p1 = static_cast<float>(apu.GetPulse1().Output());
  const auto p2 = static_cast<float>(apu.GetPulse2().Output());
  const auto t = static_cast<float>(apu.GetTriangle().Output());
  const auto n = static_cast<float>(apu.GetNoise().Output());
  REQUIRE(p1 == 5);
  REQUIRE(p2 == 3);
  REQUIRE(t == 15);
  REQUIRE(n == 5);

  apu.TakeSample();

  const float expected = 0.00752f * (p1 + p2) + 0.00851f * t + 0.00494f * n;
  REQUIRE(apu.GetSampleBuffer().size() == 1);
  CHECK(apu.GetSampleBuffer()[0] == doctest::Approx(expected));
}

TEST_CASE("DrainSamples returns accumulated samples and leaves the buffer empty") {
  nes::Apu apu;

  // CYCLES_PER_SAMPLE is ~40.58; stepping well past that guarantees at least
  // one sample has been taken.
  apu.Step(100);
  REQUIRE_FALSE(apu.GetSampleBuffer().empty());
  const auto sample_count_before = apu.GetSampleBuffer().size();

  const auto drained = apu.DrainSamples();

  CHECK(drained.size() == sample_count_before);
  CHECK(apu.GetSampleBuffer().empty());
}

TEST_CASE("Serialize followed by Deserialize round-trips the cycle counter and a channel's output") {
  nes::Apu apu;
  apu.GetPulse1().SetEnabled(true);
  apu.GetPulse1().WriteControl(0xD5); // duty 75%, constant volume 5
  apu.GetPulse1().WriteTimerLow(0x08);
  apu.GetPulse1().WriteTimerHigh(0x00);
  apu.Step(1000);
  REQUIRE(apu.GetCycle() == 1000);
  REQUIRE(apu.GetPulse1().Output() == 5);

  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);
  apu.Serialize(writer);

  nes::Apu restored;
  nes::StateReader reader(buffer);
  restored.Deserialize(reader);

  CHECK(restored.GetCycle() == 1000);
  CHECK(restored.GetPulse1().Output() == 5);
  CHECK(reader.BytesRemaining() == 0);
}

TEST_CASE("Serialize/Deserialize round-trips the frame counter's mode and in-progress position") {
  nes::Apu apu;
  apu.WriteFrameCounter(0x80); // mode 1
  apu.Step(10000); // advance partway through the 5-step sequence
  REQUIRE(apu.GetFrameMode() == 1);
  const auto frame_cycle_before = apu.GetFrameCycle();
  REQUIRE(frame_cycle_before > 0);

  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);
  apu.Serialize(writer);

  nes::Apu restored;
  nes::StateReader reader(buffer);
  restored.Deserialize(reader);

  CHECK(restored.GetFrameMode() == 1);
  CHECK(restored.GetFrameCycle() == frame_cycle_before);
}

TEST_CASE("Serialize/Deserialize round-trips the sample-rate downsampling accumulator") {
  nes::Apu apu;
  apu.Step(35); // advance close to, but not past, the ~40.58-cycle sample boundary
  REQUIRE(apu.GetSampleBuffer().empty());

  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);
  apu.Serialize(writer);

  nes::Apu restored;
  nes::StateReader reader(buffer);
  restored.Deserialize(reader);

  apu.Step(6); // crosses the sample boundary
  restored.Step(6);

  // Only matches if sample_clock_ itself round-tripped, rather than resetting to 0.
  CHECK(restored.GetSampleBuffer().size() == apu.GetSampleBuffer().size());
}

TEST_CASE("Serialize/Deserialize round-trips every channel's state") {
  nes::Apu apu;
  apu.GetPulse1().SetEnabled(true);
  apu.GetPulse1().WriteControl(0xD5); // duty 75%, constant volume 5
  apu.GetPulse1().WriteTimerLow(0x08);
  apu.GetPulse1().WriteTimerHigh(0x00);

  apu.GetPulse2().SetEnabled(true);
  apu.GetPulse2().WriteControl(0xD3); // duty 75%, constant volume 3
  apu.GetPulse2().WriteTimerLow(0x08);
  apu.GetPulse2().WriteTimerHigh(0x00);

  apu.GetTriangle().SetEnabled(true);
  apu.GetTriangle().WriteLinearCounter(0x7F);
  apu.GetTriangle().WriteTimerLow(0x02);
  apu.GetTriangle().WriteTimerHigh(0x00);
  apu.GetTriangle().ClockLinearCounter();

  apu.GetNoise().SetEnabled(true);
  apu.GetNoise().WriteControlRegister(0x15); // constant volume 5
  apu.GetNoise().WriteLengthCounterRegister(0x00);
  apu.GetNoise().ClockTimer(); // clears the power-up LFSR mute

  REQUIRE(apu.GetPulse1().Output() == 5);
  REQUIRE(apu.GetPulse2().Output() == 3);
  REQUIRE(apu.GetTriangle().Output() == 15);
  REQUIRE(apu.GetNoise().Output() == 5);

  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);
  apu.Serialize(writer);

  nes::Apu restored;
  nes::StateReader reader(buffer);
  restored.Deserialize(reader);

  CHECK(restored.GetPulse1().Output() == 5);
  CHECK(restored.GetPulse2().Output() == 3);
  CHECK(restored.GetTriangle().Output() == 15);
  CHECK(restored.GetNoise().Output() == 5);
}
