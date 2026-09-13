#pragma once

#include <cstdint>

namespace nes {

class Noise {
public:
  Noise() = default;
  ~Noise() = default;

private:
  bool enabled_{false};
  bool mode_{false};
  bool length_halt_{false};
  bool length_volume_{false};
  bool envelope_start_{false};
  uint8_t volume_{0};
  uint8_t length_counter_{0};
  uint8_t envelope_divider_{0};
  uint8_t envelope_decay_{0};
  uint16_t shift_register_{1};
  uint16_t timer_{0};
  uint16_t timer_period_{0};

  static constexpr uint16_t PERIOD_TABLE[16] = {4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 580,  726,  1016,  2034,  4086};
};

} // namespace nes