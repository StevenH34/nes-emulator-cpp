#include "TestBus.h"

#include "TestRom.h"

namespace nes_test {

TestBus::TestBus() : PpuHolder(GetTestCartridge()), nes::Bus(GetTestCartridge(), ppu) {}

} // nes_test
