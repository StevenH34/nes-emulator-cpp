#ifndef NES_EMULATOR_CPP_TEST_BUS_H
#define NES_EMULATOR_CPP_TEST_BUS_H

#include "../src/Bus.h"

namespace nes_test {

// A Bus wired to a shared, process-wide dummy Cartridge, for CPU/opcode tests that
// only exercise the RAM range and don't care about cartridge contents.
class TestBus : public nes::Bus {
public:
    TestBus();
};

} // nes_test

#endif //NES_EMULATOR_CPP_TEST_BUS_H
