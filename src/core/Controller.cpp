#include "Controller.h"

namespace nes {

/// If the strobe is active it returns the state of button A.
/// When the strobe is off, bit 0 of the shift register is returned, then the
/// register is shifted right. Unlike real hardware's open-bus behavior (which
/// reads back as 1), reads past the 8th bit here return 0 forever until the
/// next strobe.
std::uint8_t Controller::Read() {
    if (strobe_) {
        return buttons_ & 1;
    }
    const int value = shift_register_ & 1;
    shift_register_ >>= 1;
    return value;
}

/// Looks at bit 0 first.
/// If it's 1, the strobe is active and the current button state is captured into the shift register.
/// If 0, the strobe is deactivated and the state is frozen.
void Controller::Write(const std::uint8_t value) {
    strobe_ = (value & 1) != 0;
    if (strobe_) {
        shift_register_ = buttons_;
    }
}


} // namespace nes
