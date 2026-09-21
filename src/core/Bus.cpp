#include "Bus.h"
#include "save_state/StateReader.h"
#include "save_state/StateWriter.h"

#include <iostream>
#include <ostream>

namespace nes {

Bus::Bus(Cartridge& cartridge, Ppu& ppu, Apu& apu) : cartridge_(cartridge), ppu_(ppu), apu_(apu) {};

uint8_t Bus::ReadCpu(const uint16_t address) const {
  if (address >= RAM_START && address <= RAM_MIRROR_END) {
    return ReadRam(address);
  }
  if (address >= PPU_START && address <= PPU_MIRROR_END) {
    return ppu_.ReadRegister(address);
  }
  if (address >= PRG_ROM_START && address <= PRG_ROM_END) {
    return cartridge_.GetMapper().ReadPrg(address);
  }
  if (address == APU_STATUS) {
    return apu_.ReadStatus();
  }
  if (address == CONTROLLER_1) {
    return controller_1_.Read();
  }
  if (address == CONTROLLER_2) {
    return controller_2_.Read();
  }

  return 0;
}

void Bus::WriteCpu(const uint16_t address, const uint8_t value) {
  if (address >= RAM_START && address <= RAM_MIRROR_END) {
    WriteRam(address, value);
  }
  if (address >= PPU_START && address <= PPU_MIRROR_END) {
    ppu_.WriteRegister(address, value);
  }
  if (address == OAM_DMA) {
    OamDma(value);
  }
  if (address == CONTROLLER_1) {
    // When the game writes to $4016, the strobe goes to both controllers.
    controller_1_.Write(value);
    controller_2_.Write(value);
  }
  if (address >= APU_START && address <= APU_END) {
    apu_.WriteRegisters(address, value);
  }
  if (address == APU_STATUS) {
    apu_.WriteStatus(value);
  }
  if (address == APU_FRAME_COUNTER) {
    apu_.WriteFrameCounter(value);
  }
}

uint8_t Bus::ReadRam(const uint16_t address) const {
  const auto mirrored_address = static_cast<std::size_t>(address & RAM_MASK);
  return ram_[mirrored_address];
}

void Bus::WriteRam(const uint16_t address, const uint8_t value) {
  const auto mirrored_address = static_cast<std::size_t>(address & RAM_MASK);
  ram_[mirrored_address] = value;
}

/// Copy 256 bytes from CPU page $XX00 to PPU OAM memory.
void Bus::OamDma(const uint8_t page) const {
  const uint16_t base = static_cast<uint16_t>(page << 8);
  std::array<uint8_t, 256> data{};
  for (size_t i = 0; i < 256; ++i) {
    data[i] = ReadCpu(static_cast<uint16_t>(base + i));
  }
  ppu_.OamDma(data);
}

void Bus::Serialize(StateWriter& writer) const {
  writer.WriteBytes(std::span<const uint8_t>(ram_.data(), ram_.size()));
  controller_1_.Serialize(writer);
  controller_2_.Serialize(writer);
}

void Bus::Deserialize(StateReader& reader) {
  reader.ReadBytes(std::span<uint8_t>(ram_.data(), ram_.size()));
  controller_1_.Deserialize(reader);
  controller_2_.Deserialize(reader);
}

} // namespace nes
