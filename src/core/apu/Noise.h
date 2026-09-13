#pragma once

#include <cstdint>

namespace nes {

class Noise {
public:
  Noise() = default;
  ~Noise() = default;

  [[nodiscard]] bool GetEnabled() const { return enabled_; }
  [[nodiscard]] uint8_t GetLengthCounter() const { return length_counter_; }

  void SetEnabled(bool enabled);

  void WriteControlRegister(uint8_t value);
  void WriteModePeriod(uint8_t value);
  void WriteLengthCounterRegister(uint8_t value);

  void ClockTimer();
  void ClockEnvelope();
  void ClockLengthCounter();

  [[nodiscard]] uint8_t Output() const;

private:
  bool enabled_{false};
  bool mode_{false};
  bool length_halt_{false};
  bool envelope_start_{false};
  bool constant_volume_{false};
  uint8_t volume_{0};
  uint8_t length_counter_{0};
  uint8_t envelope_divider_{0};
  uint8_t envelope_decay_{0};
  uint16_t shift_register_{1};
  uint16_t timer_{0};
  uint16_t timer_period_{0};

  static constexpr uint16_t PERIOD_TABLE[16] = {4,   8,   16,  32,  64,  96,   128,  160,
                                                202, 254, 380, 508, 762, 1016, 2034, 4068};

  void shift();
};

} // namespace nes