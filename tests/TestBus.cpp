#include "TestBus.h"

#include "TestRom.h"

namespace nes_test {

TestBus::TestBus() : nes::Bus(GetTestCartridge()) {}

} // nes_test
