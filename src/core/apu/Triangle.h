#pragma once
#include <cstdint>

namespace nes {

class Triangle {
public:
  Triangle() = default;
  ~Triangle() = default;

  void SetEnabled(bool enabled);

  void WriteLinearCounter(uint8_t value);
  void WriteTimerLow(uint8_t value);
  void WriteTimerHigh(uint8_t value);

  [[nodiscard]] uint8_t GetLengthCounter() const { return length_counter_; }
  [[nodiscard]] bool GetEnabled() const { return enabled_; }

  void ClockTimer();
  void ClockLinearCounter();
  void ClockLengthCounter();
  [[nodiscard]] uint8_t Output() const;

private:
  uint8_t linear_reload_{0};
  uint8_t linear_counter_{0};
  uint8_t length_counter_{0};
  uint8_t sequence_position_{0};
  uint16_t timer_{0};
  uint16_t timer_period_{0};
  bool linear_reload_flag_{false};
  bool enabled_{false};
  bool control_flag_{false}; // Also doubles as the length counter halt flag.

  /**
   * Step sequence for the Triangle wave.
   * Goes up 15 then back down 15.
   */
  static constexpr uint8_t SEQUENCE_TABLE[32] = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5,  4,  3,  2,  1,  0,
                                                 0,  1,  2,  3,  4,  5,  6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
};

} // namespace nes
