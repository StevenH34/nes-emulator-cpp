#include "Noise.h"

#include "ApuConstants.h"

namespace nes {

void Noise::SetEnabled(const bool enabled) {
  enabled_ = enabled;
  length_counter_ = enabled_ ? length_counter_ : 0;
}

// $400C: --LC VVVV
void Noise::WriteControlRegister(const uint8_t value) {
  length_halt_ = (value & 0x20) != 0;
  constant_volume_ = (value & 0x10) != 0;
  volume_ = value & 0x0F;
}

// $400E: M--- PPPP
void Noise::WriteModePeriod(const uint8_t value) {
  mode_ = (value & 0x80) != 0;
  timer_period_ = PERIOD_TABLE[value & 0x0F];
}

// $400F: LLLL L---
void Noise::WriteLengthCounterRegister(const uint8_t value) {
  if (enabled_) {
    length_counter_ = ApuConstants::LENGTH_COUNTER_TABLE[(value >> 3) & 0x1F];
  }
  envelope_start_ = true;
}

void Noise::ClockTimer() {
  if (timer_ == 0) {
    timer_ = timer_period_;
    shift();
  } else {
    --timer_;
  }
}

void Noise::ClockEnvelope() {
  if (envelope_start_) {
    envelope_start_ = false;
    envelope_decay_ = 15;
    envelope_divider_ = volume_;
  } else {
    if (envelope_divider_ == 0) {
      envelope_divider_ = volume_;
      if (envelope_decay_ > 0) {
        --envelope_decay_;
      } else if (length_halt_) {
        envelope_decay_ = 15;
      }
    } else {
      --envelope_divider_;
    }
  }
}

void Noise::ClockLengthCounter() {
  if (!length_halt_ && length_counter_ > 0) {
    --length_counter_;
  }
}

uint8_t Noise::Output() const {
  if (!enabled_ || length_counter_ == 0 || ((shift_register_ & 1) == 1)) {
    return 0;
  }
  return constant_volume_ ? volume_ : envelope_decay_;
}

void Noise::shift() {
  const uint16_t bit = mode_ ? 6 : 1;
  const uint16_t feedback = (shift_register_ & 1) ^ ((shift_register_ >> bit) & 1);
  shift_register_ >>= 1;
  shift_register_ |= (feedback << 14);
}

} // namespace nes