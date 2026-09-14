#ifndef NES_EMULATOR_CPP_TEST_BUS_H
#define NES_EMULATOR_CPP_TEST_BUS_H

#include "../src/core/Bus.h"
#include "../src/core/apu/Apu.h"
#include "../src/core/ppu/Ppu.h"

namespace nes_test {

namespace detail {
// Constructed before the nes::Bus base below (base classes construct in
// declaration order), so TestBus can hand Bus a Ppu&/Apu& that are private to
// this TestBus instance instead of a process-wide singleton shared by every test.
struct DeviceHolder {
  nes::Ppu ppu;
  nes::Apu apu;
  explicit DeviceHolder(nes::Cartridge& cartridge) : ppu(cartridge) {}
};
} // namespace detail

// A Bus wired to a shared, process-wide dummy Cartridge and a fresh, private
// Ppu/Apu, for CPU/opcode tests that only exercise the RAM range and don't care
// about cartridge, PPU, or APU state.
class TestBus : private detail::DeviceHolder, public nes::Bus {
public:
  TestBus();
};

} // namespace nes_test

#endif // NES_EMULATOR_CPP_TEST_BUS_H
