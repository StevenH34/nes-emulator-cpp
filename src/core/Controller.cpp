#include "Controller.h"
#include "./save_state/StateReader.h"
#include "./save_state/StateWriter.h"

namespace nes {
/**
 * If the strobe is active it returns the state of button A.
 * When the strobe is off, bit 0 of the shift register is returned, then the
 * register is shifted right. Unlike real hardware's open-bus behavior (which
 * reads back as 1), reads past the 8th bit here return 0 forever until the
 * next strobe.
 */
uint8_t Controller::Read() {
  if (strobe_) {
    return buttons_ & 1;
  }
  const uint8_t value = shift_register_ & 1;
  shift_register_ >>= 1;
  return value;
}
/**
 * Looks at bit 0 first.
 * If it's 1, the strobe is active and the current button state is captured
 * into the shift register. If 0, the strobe is deactivated and the state is
 * frozen. Real hardware reloads the register continuously while the strobe is
 * high, so a 0 write that ends a high strobe also captures the current button
 * state before freezing it.
 */
void Controller::Write(const uint8_t value) {
  if (strobe_ || (value & 1))
    shift_register_ = buttons_;
  strobe_ = (value & 1) != 0;
}

// Save and load state
void Controller::Serialize(StateWriter& writer) const {
  writer.WriteU8(buttons_);
  writer.WriteU8(shift_register_);
  writer.WriteBool(strobe_);
}

void Controller::Deserialize(StateReader& reader) {
  buttons_ = reader.ReadU8();
  shift_register_ = reader.ReadU8();
  strobe_ = reader.ReadBool();
}

} // namespace nes
