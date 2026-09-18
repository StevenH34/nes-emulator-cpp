#include "Noise.h"

#include "ApuConstants.h"
#include "./save_state/StateReader.h"
#include "./save_state/StateWriter.h"

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

void Noise::Serialize(StateWriter& writer) const {
  writer.WriteBool(enabled_);
  writer.WriteBool(mode_);
  writer.WriteBool(length_halt_);
  writer.WriteBool(envelope_start_);
  writer.WriteBool(constant_volume_);
  writer.WriteU8(volume_);
  writer.WriteU8(length_counter_);
  writer.WriteU8(envelope_divider_);
  writer.WriteU8(envelope_decay_);
  writer.WriteU16(shift_register_);
  writer.WriteU16(timer_);
  writer.WriteU16(timer_period_);
}

void Noise::Deserialize(StateReader& reader) {
  enabled_ = reader.ReadBool();
  mode_ = reader.ReadBool();
  length_halt_ = reader.ReadBool();
  envelope_start_ = reader.ReadBool();
  constant_volume_ = reader.ReadBool();
  volume_ = reader.ReadU8();
  length_counter_ = reader.ReadU8();
  envelope_divider_ = reader.ReadU8();
  envelope_decay_ = reader.ReadU8();
  shift_register_ = reader.ReadU16();
  timer_ = reader.ReadU16();
  timer_period_ = reader.ReadU16();
}

} // namespace nes