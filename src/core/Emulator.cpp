#include "Emulator.h"
#include "save_state/StateReader.h"
#include "save_state/StateWriter.h"

#include <array>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <system_error>

namespace nes::SaveStateFormat {
inline constexpr std::array<uint8_t, 4> MAGIC{'N', 'E', 'S', 'S'};
inline constexpr uint16_t VERSION = 1;
} // namespace nes::SaveStateFormat

namespace nes {

Emulator::Emulator(std::string path)
    : cartridge_(std::move(path)), ppu_(cartridge_), bus_(cartridge_, ppu_, apu_), cpu_(bus_) {
  ppu_.SetNmiCallback([this] { cpu_.Nmi(); });
  cpu_.Reset();
}

Emulator::Emulator(const std::span<const uint8_t> rom_bytes)
    : cartridge_(rom_bytes), ppu_(cartridge_), bus_(cartridge_, ppu_, apu_), cpu_(bus_) {
  ppu_.SetNmiCallback([this] { cpu_.Nmi(); });
  cpu_.Reset();
}

int Emulator::Step() {
  // Ticks the CPU forward
  const int cycles = cpu_.Step();
  for (size_t i = 0; i < static_cast<size_t>(cycles) * 3; ++i) {
    ppu_.Step();
  }
  apu_.Step(cycles);
  return cycles;
}

const std::vector<uint8_t>& Emulator::RunFrame() {
  if (ppu_.IsFrameComplete()) {
    ppu_.ClearFrameComplete();
  }
  while (!ppu_.IsFrameComplete()) {
    Step();
  }
  return ppu_.GetFrameBuffer();
}

void Emulator::LoadProgram(const std::vector<uint8_t>& program, const uint16_t start_address) {
  uint16_t address = start_address;

  for (const auto byte : program) {
    bus_.WriteCpu(address, byte);
    ++address;
  }
}

void Emulator::Serialize(StateWriter& writer) const {
  cpu_.Serialize(writer);
  ppu_.Serialize(writer);
  apu_.Serialize(writer);
  bus_.Serialize(writer);
  cartridge_.Serialize(writer);
}

void Emulator::Deserialize(StateReader& reader) {
  // Snapshot current good state to avoid truncated/corrupt payload
  std::vector<uint8_t> snapshot;
  StateWriter snapshot_writer(snapshot);
  Serialize(snapshot_writer);

  try {
    cpu_.Deserialize(reader);
    ppu_.Deserialize(reader);
    apu_.Deserialize(reader);
    bus_.Deserialize(reader);
    cartridge_.Deserialize(reader);
  } catch (...) {
    StateReader snapshot_reader(snapshot);
    cpu_.Deserialize(snapshot_reader);
    ppu_.Deserialize(snapshot_reader);
    apu_.Deserialize(snapshot_reader);
    bus_.Deserialize(snapshot_reader);
    cartridge_.Deserialize(snapshot_reader);
    throw;
  }

  ppu_.SetNmiCallback([this] { cpu_.Nmi(); });
  apu_.GetSampleBuffer().clear();
}

void Emulator::SaveStateToFile(const std::string& path) const {
  const std::vector<uint8_t> data = SaveStateToBytes();

  // Write to a temp file and rename it over the target, so a failed save leaves the old file intact
  const std::string temp_path = path + ".tmp";
  {
    std::ofstream file_stream(temp_path, std::ios::binary);
    if (!file_stream) {
      throw std::runtime_error(std::format("Failed to open file for writing: {}", path));
    }
    file_stream.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    // Close explicitly so a failed final flush is reported instead of ignored by the destructor
    file_stream.close();
    if (!file_stream) {
      std::error_code ignored;
      std::filesystem::remove(temp_path, ignored);
      throw std::runtime_error(std::format("Failed to write to file: {}", path));
    }
  }

  std::error_code ec;
  std::filesystem::rename(temp_path, path, ec);
  if (ec) {
    std::error_code ignored;
    std::filesystem::remove(temp_path, ignored);
    throw std::runtime_error(std::format("Failed to replace {}: {}", path, ec.message()));
  }
}

void Emulator::LoadStateFromFile(const std::string& path) {
  const std::vector<uint8_t> data = Cartridge::ReadFileBytes(path);
  LoadStateFromBytes(data);
}

// Build save state in memory with header + payload.
std::vector<uint8_t> Emulator::SaveStateToBytes() const {
  std::vector<uint8_t> payload;
  StateWriter payload_writer(payload);
  Serialize(payload_writer);

  std::vector<uint8_t> data;
  StateWriter writer(data);
  writer.WriteBytes(SaveStateFormat::MAGIC);
  writer.WriteU16(SaveStateFormat::VERSION);
  writer.WriteU8(cartridge_.GetMapperId());
  writer.WriteU32(static_cast<uint32_t>(cartridge_.GetPrgRom().size()));
  writer.WriteU32(static_cast<uint32_t>(cartridge_.GetChrRom().size()));
  writer.WriteU32(cartridge_.GetRomChecksum());
  writer.WriteU32(static_cast<uint32_t>(payload.size()));
  writer.WriteBytes(payload);

  return data;
}

// Validate save state header against currently loaded ROM before restoring.
void Emulator::LoadStateFromBytes(const std::span<const uint8_t> bytes) {
  StateReader reader(bytes);

  try {
    std::array<uint8_t, 4> magic{};
    reader.ReadBytes(magic);
    if (magic != SaveStateFormat::MAGIC) {
      throw std::runtime_error("Invalid save state data: incorrect magic number");
    }
    if (reader.ReadU16() != SaveStateFormat::VERSION) {
      throw std::runtime_error("Invalid save state data: unsupported version");
    }

    const uint8_t mapper_id = reader.ReadU8();
    const uint32_t prg_rom_size = reader.ReadU32();
    const uint32_t chr_rom_size = reader.ReadU32();
    const uint32_t rom_checksum = reader.ReadU32();
    if (mapper_id != cartridge_.GetMapperId() || prg_rom_size != cartridge_.GetPrgRom().size() ||
        chr_rom_size != cartridge_.GetChrRom().size() || rom_checksum != cartridge_.GetRomChecksum()) {
      throw std::runtime_error("Save state data does not match the current game");
    }
    // Reject truncated or padded payloads before any component state is touched
    const uint32_t payload_size = reader.ReadU32();
    if (payload_size != reader.BytesRemaining()) {
      throw std::runtime_error("Save state data is corrupt or truncated: payload size mismatch");
    }
    Deserialize(reader);
  } catch (const std::out_of_range& e) {
    throw std::runtime_error(std::format("Save state data is corrupt or truncated: {}", e.what()));
  }
}

} // namespace nes