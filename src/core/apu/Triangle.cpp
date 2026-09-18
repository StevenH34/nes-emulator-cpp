#include "Triangle.h"

#include "./save_state/StateReader.h"
#include "./save_state/StateWriter.h"
#include "ApuConstants.h"

namespace nes {

void Triangle::SetEnabled(const bool enabled) {
  enabled_ = enabled;
  if (!enabled_)
    length_counter_ = 0;
}

// $4008: CRRR RRRR
void Triangle::WriteLinearCounter(const uint8_t value) {
  control_flag_ = (value & 0x80) != 0;
  linear_reload_ = value & 0x7F;
}

// $400A: TTTT TTTT
void Triangle::WriteTimerLow(const uint8_t value) {
  timer_period_ = (timer_period_ & 0x0700) | static_cast<uint16_t>(value);
}

// $400B: LLLL LTTT
void Triangle::WriteTimerHigh(const uint8_t value) {
  timer_period_ = static_cast<uint16_t>((timer_period_ & 0x00FF) | (static_cast<uint16_t>(value & 0x07) << 8));
  if (enabled_)
    length_counter_ = ApuConstants::LENGTH_COUNTER_TABLE[(value >> 3) & 0x1F];
  linear_reload_flag_ = true;
}

// Clocked every CPU cycle.
void Triangle::ClockTimer() {
  if (timer_ == 0) {
    timer_ = timer_period_;
    if (length_counter_ > 0 && linear_counter_ > 0) {
      sequence_position_ = (sequence_position_ + 1) & 0x1F;
    }
  } else {
    --timer_;
  }
}

/**
 * Clocks every quarter frame.
 * If the reload flag is set, load the reload value.
 */
void Triangle::ClockLinearCounter() {
  if (linear_reload_flag_) {
    linear_counter_ = linear_reload_;
  } else if (linear_counter_ > 0) {
    --linear_counter_;
  }
  if (!control_flag_) {
    linear_reload_flag_ = false;
  }
}

void Triangle::ClockLengthCounter() {
  if (!control_flag_ && length_counter_ > 0) {
    --length_counter_;
  }
}

uint8_t Triangle::Output() const {
  if (!enabled_)
    return 0;
  if (length_counter_ == 0)
    return 0;
  if (linear_counter_ == 0)
    return 0;
  if (timer_period_ < 2)
    return 0; // Silence ultrasonic frequencies

  return SEQUENCE_TABLE[sequence_position_];
}

// Save and load state
void Triangle::Serialize(StateWriter& writer) const {
  writer.WriteU8(linear_reload_);
  writer.WriteU8(linear_counter_);
  writer.WriteU8(length_counter_);
  writer.WriteU8(sequence_position_);
  writer.WriteU16(timer_);
  writer.WriteU16(timer_period_);
  writer.WriteBool(linear_reload_flag_);
  writer.WriteBool(enabled_);
  writer.WriteBool(control_flag_);
}
void Triangle::Deserialize(StateReader& reader) {
  linear_reload_ = reader.ReadU8();
  linear_counter_ = reader.ReadU8();
  length_counter_ = reader.ReadU8();
  sequence_position_ = reader.ReadU8();
  timer_ = reader.ReadU16();
  timer_period_ = reader.ReadU16();
  linear_reload_flag_ = reader.ReadBool();
  enabled_ = reader.ReadBool();
  control_flag_ = reader.ReadBool();
}

} // namespace nes
