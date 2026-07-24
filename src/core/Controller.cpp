#include "Controller.h"

namespace nes {

std::uint8_t Controller::Read() {
    if (strobe_) {
        return buttons_ & 1;
    }
    const int value = shift_register_ & 1;
    shift_register_ >>= 1;
    return value;
}

void Controller::Write(std::uint8_t value) {
    strobe_ = (value & 1) != 0;
    if (strobe_) {
        shift_register_ = buttons_;
    }
}


} // namespace nes
