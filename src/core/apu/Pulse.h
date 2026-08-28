#pragma once

#include <cstdint>

namespace nes {

class Pulse {
public:
  explicit Pulse(uint8_t channel);
  ~Pulse() = default;

  void SetEnabled(bool enabled);

  void WriteControl(uint8_t value);
  void WriteTimerHigh(uint8_t value);
  void WriteTimerLow(uint8_t value);
  void WriteSweep(uint8_t value);

  [[nodiscard]] uint8_t GetLengthCounter() const { return length_counter_; }
  [[nodiscard]] bool GetEnabled() const { return enabled_; }
  [[nodiscard]] uint16_t GetTimerPeriod() const { return timer_period_; }

  void ClockTimer();
  void ClockEnvelope();
  void ClockLengthCounter();
  void ClockSweep();
  [[nodiscard]] uint8_t Output() const;


private:
  uint8_t channel_;
  uint8_t volume_{0};
  uint8_t length_counter_{0};
  uint8_t duty_cycle_{0};
  uint8_t duty_step_{0};
  uint8_t envelope_decay_{0};
  uint8_t envelope_divider_{0};
  uint8_t sweep_divider_{0};
  uint8_t sweep_period_{0};
  uint8_t sweep_shift_{0};
  uint16_t timer_period_{0};
  uint16_t timer_{0};
  bool enabled_{false};
  bool length_halt_{false};
  bool constant_value_{false};
  bool envelope_start_{false};
  bool sweep_enabled_{false};
  bool sweep_negate_{false};
  bool sweep_reload_{false};

  static constexpr uint8_t DUTY_TABLE_[4][8] = {
    {0, 1, 0, 0, 0, 0, 0, 0}, // 12.5%
    {0, 1, 1, 0, 0, 0, 0, 0}, // 25%
    {0, 1, 1, 1, 1, 0, 0, 0}, // 50%
    {1, 0, 0, 1, 1, 1, 1, 1}, // 75%
  };

  [[nodiscard]] uint16_t SweepTargetPeriod() const;
};

} // namespace nes