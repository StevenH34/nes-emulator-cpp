#include "InputRecording.h"
#include "Emulator.h"
#include "save_state/StateReader.h"
#include "save_state/StateWriter.h"

#include <array>
#include <format>
#include <fstream>
#include <stdexcept>

namespace nes::RecordingFormat {
inline constexpr std::array<uint8_t, 4> MAGIC{'N', 'E', 'S', 'R'};
inline constexpr uint16_t VERSION = 1;
} // namespace nes::RecordingFormat

namespace nes {

std::vector<uint8_t> RecordingToBytes(const Recording& recording) {
  std::vector<uint8_t> data;
  StateWriter writer(data);
  writer.WriteBytes(RecordingFormat::MAGIC);
  writer.WriteU16(RecordingFormat::VERSION);
  writer.WriteU32(recording.rom_checksum);
  writer.WriteU32(static_cast<uint32_t>(recording.start_state.size()));
  writer.WriteBytes(recording.start_state);
  writer.WriteU32(static_cast<uint32_t>(recording.buttons.size()));
  writer.WriteBytes(recording.buttons);
  return data;
}

Recording RecordingFromBytes(const std::span<const uint8_t> bytes) {
  StateReader reader(bytes);

  try {
    std::array<uint8_t, 4> magic{};
    reader.ReadBytes(magic);
    if (magic != RecordingFormat::MAGIC) {
      throw std::runtime_error("Invalid recording data: incorrect magic number");
    }
    if (reader.ReadU16() != RecordingFormat::VERSION) {
      throw std::runtime_error("Invalid recording data: unsupported version");
    }

    Recording recording;
    recording.rom_checksum = reader.ReadU32();
    // Check the size before allocating so junk data can't request a huge buffer
    const uint32_t state_size = reader.ReadU32();
    if (state_size > reader.BytesRemaining()) {
      throw std::runtime_error("Recording data is corrupt or truncated: start state size exceeds data");
    }
    recording.start_state.resize(state_size);
    reader.ReadBytes(recording.start_state);
    // Reject truncated or padded button data
    const uint32_t frame_count = reader.ReadU32();
    if (frame_count != reader.BytesRemaining()) {
      throw std::runtime_error("Recording data is corrupt or truncated: frame count mismatch");
    }
    recording.buttons.resize(frame_count);
    reader.ReadBytes(recording.buttons);
    return recording;
  } catch (const std::out_of_range& e) {
    throw std::runtime_error(std::format("Recording data is corrupt or truncated: {}", e.what()));
  }
}

void SaveRecording(const Recording& recording, const std::string& path) {
  const std::vector<uint8_t> data = RecordingToBytes(recording);
  std::ofstream file_stream(path, std::ios::binary);
  if (!file_stream) {
    throw std::runtime_error(std::format("Failed to open file for writing: {}", path));
  }
  file_stream.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
  // Close explicitly so a failed final flush is reported instead of ignored by the destructor
  file_stream.close();
  if (!file_stream) {
    throw std::runtime_error(std::format("Failed to write to file: {}", path));
  }
}

Recording LoadRecording(const std::string& path) { return RecordingFromBytes(Cartridge::ReadFileBytes(path)); }

void Recorder::Start(Emulator& emulator) {
  if (recording_) {
    throw std::logic_error("Recorder::Start: already recording");
  }
  recording_ = Recording{emulator.GetCartridge().GetRomChecksum(), emulator.SaveStateToBytes(), {}};
}

void Recorder::RecordFrame(const uint8_t buttons) {
  if (!recording_) {
    throw std::logic_error("Recorder::RecordFrame: not recording");
  }
  recording_->buttons.push_back(buttons);
}

void Recorder::Finish(const std::string& path) {
  if (!recording_) {
    throw std::logic_error("Recorder::Finish: not recording");
  }
  SaveRecording(*recording_, path);
  recording_.reset();
}

} // namespace nes
