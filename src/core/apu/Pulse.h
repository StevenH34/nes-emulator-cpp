#pragma once

#include <cstdint>

namespace nes {
  class Pulse {
  public:

  private:
    static constexpr std::uint8_t DUTY_TABLE_[4][8] = {
      {0, 1, 0, 0, 0, 0, 0, 0},
      {0, 1, 1, 0, 0, 0, 0, 0},
      {0, 1, 1, 1, 1, 0, 0, 0},
      {1, 0, 0, 1, 1, 1, 1, 1},
    };
};

} // namespace nes