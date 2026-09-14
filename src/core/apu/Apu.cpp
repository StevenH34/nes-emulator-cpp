#include "Apu.h"

#include <cstdint>

namespace nes {

Apu::Apu() : pulse1_{0}, pulse2_{1}, triangle_{}, noise_{} { sample_buffer_.reserve(800); }

void Apu::ClockFrameCounter() {
  ++frame_cycle_;

  bool quarter = false;
  bool half = false;

  if (frame_mode_ == 0) { // 4-step
    switch (frame_cycle_) {
    case FrameCounterNtsc::MODE0_STEP1:
      quarter = true;
      break;
    case FrameCounterNtsc::MODE0_STEP2:
      quarter = true;
      half = true;
      break;
    case FrameCounterNtsc::MODE0_STEP3:
      quarter = true;
      break;
    case FrameCounterNtsc::MODE0_STEP4:
      quarter = true;
      half = true;
      frame_cycle_ = 0;
      break;
    default:
      break;
    }
  } else { // 5-step
    switch (frame_cycle_) {
    case FrameCounterNtsc::MODE1_STEP1:
      quarter = true;
      break;
    case FrameCounterNtsc::MODE1_STEP2:
      quarter = true;
      half = true;
      break;
    case FrameCounterNtsc::MODE1_STEP3:
      quarter = true;
      break;
    case FrameCounterNtsc::MODE1_STEP5:
      quarter = true;
      half = true;
      frame_cycle_ = 0;
      break;
    default:
      break;
    }
  }

  if (quarter)
    ClockQuarterFrame();
  if (half)
    ClockHalfFrame();
}

void Apu::ClockQuarterFrame() {
  pulse1_.ClockEnvelope();
  pulse2_.ClockEnvelope();
  triangle_.ClockLinearCounter();
  noise_.ClockEnvelope();
}

void Apu::ClockHalfFrame() {
  pulse1_.ClockLengthCounter();
  pulse1_.ClockSweep();
  pulse2_.ClockLengthCounter();
  pulse2_.ClockSweep();
  triangle_.ClockLengthCounter();
  noise_.ClockLengthCounter();
}

void Apu::TakeSample() {
  const float pulse1 = static_cast<float>(pulse1_.Output());
  const float pulse2 = static_cast<float>(pulse2_.Output());
  const float triangle = static_cast<float>(triangle_.Output());
  const float noise = static_cast<float>(noise_.Output());
  const float pulse_output = PULSE_MIXER_COEFFICIENT * (pulse1 + pulse2);
  const float triangle_noise_dmc = TND_T_MIXER_COEFFICIENT * triangle + TND_N_MIXER_COEFFICIENT * noise;

  sample_buffer_.push_back(pulse_output + triangle_noise_dmc);
}

std::vector<float> Apu::DrainSamples() {
  std::vector<float> samples;
  samples.reserve(800);
  std::swap(samples, sample_buffer_);
  return samples;
}

void Apu::WriteFrameCounter(uint8_t value) {
  frame_mode_ = (value >> 7) & 1;
  frame_cycle_ = 0;
  // Mode 1
  if (frame_mode_ == 1) {
    ClockQuarterFrame();
    ClockHalfFrame();
  }
}

// 4015: enable/disable channels
void Apu::WriteStatus(const uint8_t value) {
  pulse1_.SetEnabled((value & STATUS_PULSE1_MASK) != 0);
  pulse2_.SetEnabled((value & STATUS_PULSE2_MASK) != 0);
  triangle_.SetEnabled((value & STATUS_TRIANGLE_MASK) != 0);
  noise_.SetEnabled((value & STATUS_NOISE_MASK) != 0);
}

// $4015: read channel status
uint8_t Apu::ReadStatus() const {
  uint8_t status = 0;
  if (pulse1_.GetLengthCounter() > 0)
    status |= STATUS_PULSE1_MASK;
  if (pulse2_.GetLengthCounter() > 0)
    status |= STATUS_PULSE2_MASK;
  if (triangle_.GetLengthCounter() > 0)
    status |= STATUS_TRIANGLE_MASK;
  if (noise_.GetLengthCounter() > 0)
    status |= STATUS_NOISE_MASK;
  return status;
}

void Apu::Step(int32_t cpu_cycles) {
  for (int32_t i = 0; i < cpu_cycles; ++i) {
    triangle_.ClockTimer(); // Triangle clocks every CPU cycle
    // Pulse and Noise clock every other CPU cycles
    if (cycle_ % 2 == 0) {
      pulse1_.ClockTimer();
      pulse2_.ClockTimer();
      noise_.ClockTimer();
    }

    ClockFrameCounter();

    // Downsampling to 44.1 kHz
    sample_clock_ += 1.0f;
    if (sample_clock_ >= CYCLES_PER_SAMPLE) {
      sample_clock_ -= CYCLES_PER_SAMPLE;
      TakeSample();
    }

    ++cycle_;
  }
}

void Apu::WriteRegisters(const uint16_t address, const uint8_t value) {
  switch (address) {
  case PULSE1_MAIN_REGISTER:
    pulse1_.WriteControl(value);
    break;
  case PULSE1_SWEEP_REGISTER:
    pulse1_.WriteSweep(value);
    break;
  case PULSE1_TIMER_LOW_REGISTER:
    pulse1_.WriteTimerLow(value);
    break;
  case PULSE1_TIMER_HIGH_REGISTER:
    pulse1_.WriteTimerHigh(value);
    break;
  case PULSE2_MAIN_REGISTER:
    pulse2_.WriteControl(value);
    break;
  case PULSE2_SWEEP_REGISTER:
    pulse2_.WriteSweep(value);
    break;
  case PULSE2_TIMER_LOW_REGISTER:
    pulse2_.WriteTimerLow(value);
    break;
  case PULSE2_TIMER_HIGH_REGISTER:
    pulse2_.WriteTimerHigh(value);
    break;
  case TRIANGLE_LINEAR_REGISTER:
    triangle_.WriteLinearCounter(value);
    break;
  case TRIANGLE_TIMER_LOW_REGISTER:
    triangle_.WriteTimerLow(value);
    break;
  case TRIANGLE_TIMER_HIGH_REGISTER:
    triangle_.WriteTimerHigh(value);
    break;
  case NOISE_MAIN_REGISTER:
    noise_.WriteControlRegister(value);
    break;
  case NOISE_MODE_PERIOD_REGISTER:
    noise_.WriteModePeriod(value);
    break;
  case NOISE_LENGTH_REGISTER:
    noise_.WriteLengthCounterRegister(value);
    break;
    // $4009, $400D: unused
    // $4010-$4013: DMC (not implemented)
  default:
    break;
  }
}

} // namespace nes
