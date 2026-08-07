#pragma once

#include <array>
#include <cstdint>

#include "Cartridge.h"
#include "Controller.h"
#include "ppu/Ppu.h"

namespace nes {

class Bus {
public:
  explicit Bus(Cartridge& cartridge, Ppu& ppu);
  ~Bus() = default;
  Bus(const Bus&) = delete;
  Bus& operator=(const Bus&) = delete;

  Controller& GetController1() const { return controller_1_; }
  Controller& GetController2() const { return controller_2_; }

  [[nodiscard]] uint8_t ReadCpu(uint16_t address) const;
  void WriteCpu(uint16_t address, uint8_t value);
  [[nodiscard]] uint8_t ReadRam(uint16_t address) const;
  void WriteRam(uint16_t address, uint8_t value);

  void OamDma(uint8_t page) const;

private:
  // std::vector<uint8_t> ram_;
  std::array<uint8_t, 2048> ram_{};
  Cartridge& cartridge_;
  Ppu& ppu_;
  mutable Controller controller_1_;
  mutable Controller controller_2_;

  /// CPU RAM
  static constexpr uint16_t RAM_SIZE = 2048; // 2 KB
  static constexpr uint16_t RAM_START = 0x0000;
  static constexpr uint16_t RAM_END = 0x07FF;
  static constexpr uint16_t RAM_MIRROR_END = 0x1FFF;
  static constexpr uint16_t RAM_MASK = 0x07FF;
  /// PRG-ROM Cartridge - covers the 32KB of address space for the cartridge
  /// code
  static constexpr uint16_t PRG_ROM_START = 0x8000;
  static constexpr uint16_t PRG_ROM_END = 0xFFFF;
  /// PPU Registers
  static constexpr uint16_t PPU_START = 0x2000;
  static constexpr uint16_t PPU_END = 0x2007; // 8 registers
  static constexpr uint16_t PPU_MIRROR_END = 0x3FFF; // Mirrored every 8 bytes across 8KB
  /// IO Registers
  static constexpr uint16_t OAM_DMA = 0x4014;
  /// Controllers
  static constexpr uint16_t CONTROLLER_1 = 0x4016;
  static constexpr uint16_t CONTROLLER_2 = 0x4017;
};

} // namespace nes
