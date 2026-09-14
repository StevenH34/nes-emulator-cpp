#include "TestBus.h"

#include "TestRom.h"

namespace nes_test {

TestBus::TestBus() : DeviceHolder(GetTestCartridge()), nes::Bus(GetTestCartridge(), ppu, apu) {}

} // namespace nes_test
