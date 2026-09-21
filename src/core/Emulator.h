#pragma once

#include "Bus.h"
#include "apu/Apu.h"
#include "cpu/Cpu.h"

#include <vector>

namespace nes {

class StateReader;
class StateWriter;

class Emulator {
public:
  explicit Emulator(std::string path);
  ~Emulator() = default;
  // Disable copy and move operations
  Emulator(const Emulator&) = delete;
  Emulator& operator=(const Emulator&) = delete;
  Emulator(Emulator&&) = delete;
  Emulator& operator=(Emulator&&) = delete;

  static void Run();
  int Step();
  const std::vector<uint8_t>& RunFrame();

  // For loading test programs
  void LoadProgram(const std::vector<uint8_t>& program, uint16_t start_address = 0x0000);

  // Test helpers for inspecting internal state directly
  [[nodiscard]] Bus& GetBus() { return bus_; }
  [[nodiscard]] Cpu& GetCpu() { return cpu_; }
  [[nodiscard]] Apu& GetApu() { return apu_; }
  [[nodiscard]] Ppu& GetPpu() { return ppu_; }
  [[nodiscard]] Cartridge& GetCartridge() { return cartridge_; }

  // Save and load state
  void Serialize(StateWriter& writer) const;
  void Deserialize(StateReader& reader);
  void SaveStateToFile(const std::string& path) const;
  void LoadStateFromFile(const std::string& path);

private:
  Cartridge cartridge_;
  Ppu ppu_;
  Apu apu_;
  Bus bus_;
  Cpu cpu_;
};

} // namespace nes
