#include "Pulse.h"
#include "ApuConstants.h"

namespace nes {

Pulse::Pulse(const uint8_t channel) : channel_{channel} {}

void Pulse::SetEnabled(const bool enabled) {
  enabled_ = enabled;
  if (!enabled_)
    length_counter_ = 0;
}

// $4000/$4004: DDLC VVVV
void Pulse::WriteControl(const uint8_t value) {
  duty_cycle_ = (value >> 6) & 0x03;
  length_halt_ = (value & 0x20) != 0;
  constant_value_ = (value & 0x10) != 0;
  volume_ = value & 0x0F;
}

// $4003/$4007: LLLL LTTT (length load + timer high)
void Pulse::WriteTimerHigh(const uint8_t value) {
  timer_period_ = (timer_period_ & 0x00FF) | (static_cast<uint16_t>(value & 0x07) << 8);
  if (enabled_)
    length_counter_ = ApuConstants::LENGTH_COUNTER_TABLE[(value >> 3) & 0x1F];
  duty_step_ = 0;
  envelope_start_ = true;
}

// $4002/$4006: TTTT TTTT (timer low)
void Pulse::WriteTimerLow(const uint8_t value) {
  timer_period_ = (timer_period_ & 0x0700) | static_cast<uint16_t>(value);
}

// $4001/$4005: EPPP NSSS
void Pulse::WriteSweep(const uint8_t value) {
  sweep_enabled_ = (value & 0x80) != 0;
  sweep_period_ = (value >> 4) & 0x07;
  sweep_negate_ = (value & 0x08) != 0;
  sweep_shift_ = value & 0x07;
  sweep_reload_ = true;
}

// When the timer hits 0, the duty cycle position advances.
void Pulse::ClockTimer() {
  if (timer_ == 0) {
    timer_ = timer_period_;
    duty_step_ = (duty_step_ + 1) & 0x07;
  } else {
    --timer_;
  }
}

void Pulse::ClockEnvelope() {
  if (envelope_start_) {
    envelope_start_ = false;
    envelope_decay_ = 15;
    envelope_divider_ = volume_;
  } else {
    if (envelope_divider_ == 0) {
      envelope_divider_ = volume_;
      if (envelope_decay_ > 0)
        --envelope_decay_;
      else if (length_halt_)
        envelope_decay_ = 15;
    } else {
      --envelope_divider_;
    }
  }
}

void Pulse::ClockLengthCounter() {
  if (!length_halt_ && length_counter_ > 0) {
    --length_counter_;
  }
}

// The output volume. 0 is silence.
uint8_t Pulse::Output() const {
  if (!enabled_ || length_counter_ == 0)
    return 0;
  if (DUTY_TABLE_[duty_cycle_][duty_step_] == 0)
    return 0;
  if (timer_period_ < 8 || SweepTargetPeriod() > 0x7FF)
    return 0;

  return constant_value_ ? volume_ : envelope_decay_;
}

void Pulse::ClockSweep() {
  if (const auto target = SweepTargetPeriod(); sweep_divider_ == 0 && sweep_enabled_ && sweep_shift_ > 0 && timer_period_ >= 8 && target <= 0x7FF) {
    timer_period_ = target;
  }

  if (sweep_divider_ == 0 || sweep_reload_) {
    sweep_divider_ = sweep_period_;
    sweep_reload_ = false;
  } else {
    --sweep_divider_;
  }
}

uint16_t Pulse::SweepTargetPeriod() const {
  const auto change = timer_period_ >> sweep_shift_;

  if (sweep_negate_) {
    if (channel_ == 0) {
      // Pulse 1: one's complement
      return static_cast<uint16_t>(timer_period_ - change - 1);
    }       // Pulse 2: two's complement
      return static_cast<uint16_t>(timer_period_ - change);
  }
  return static_cast<uint16_t>(timer_period_ + change);
}

} // namespace nes