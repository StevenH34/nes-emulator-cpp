#ifndef NES_EMULATOR_CPP_CONTROLLER_H
#define NES_EMULATOR_CPP_CONTROLLER_H

#include <cstdint>

namespace nes {

class Controller {
public:
    explicit Controller() = default;
    ~Controller() = default;

    /// Button masks
    // Each button is 1 bit in a byte.
    static constexpr std::uint8_t BUTTON_A = 0x01;
    static constexpr std::uint8_t BUTTON_B = 0x02;
    static constexpr std::uint8_t BUTTON_SELECT = 0x04;
    static constexpr std::uint8_t BUTTON_START  = 0x08;
    static constexpr std::uint8_t BUTTON_UP     = 0x10;
    static constexpr std::uint8_t BUTTON_DOWN   = 0x20;
    static constexpr std::uint8_t BUTTON_LEFT   = 0x40;
    static constexpr std::uint8_t BUTTON_RIGHT  = 0x80;

    void Press(const std::uint8_t button) { buttons_ |= button; }
    void Release(const std::uint8_t button) { buttons_ &= ~button; }
    std::uint8_t Read();
    void Write(std::uint8_t value);


private:
    // Current button state
    std::uint8_t buttons_{0x00};
    // Frozen snapshot of inputs, that the CPU reads
    std::uint8_t shift_register_{0x00};
    // Controls when to freeze or update the snapshot
    bool strobe_{false};


};

} // namespace nes

#endif //NES_EMULATOR_CPP_CONTROLLER_H
