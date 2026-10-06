#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace nes {

class Emulator;

/* A recorded play session: a save state to start from, plus the controller-1
 * button mask for every frame after it. Replaying the buttons from the start
 * state reproduces the session exactly.
 */
struct Recording {
  uint32_t rom_checksum{};
  std::vector<uint8_t> start_state; // Emulator::SaveStateToBytes() output
  std::vector<uint8_t> buttons;     // one controller-1 mask per frame
};

// .nesdemo file format (little-endian):
// "NESR" | u16 version | u32 rom_checksum | u32 state_size | state bytes | u32 frame_count | frame_count button bytes
[[nodiscard]] std::vector<uint8_t> RecordingToBytes(const Recording& recording);
// Throws std::runtime_error if the data is corrupt, truncated or an unsupported version.
// The start state itself is validated when it is loaded into an Emulator.
[[nodiscard]] Recording RecordingFromBytes(std::span<const uint8_t> bytes);
void SaveRecording(const Recording& recording, const std::string& path);
[[nodiscard]] Recording LoadRecording(const std::string& path);

// Builds a Recording one frame at a time. Calls in the wrong state throw std::logic_error.
class Recorder {
public:
  // Takes the start state and ROM checksum from the emulator's current state.
  void Start(Emulator& emulator);
  void RecordFrame(uint8_t buttons);
  // Writes the file and returns to idle. If writing throws, the recording is kept.
  void Finish(const std::string& path);
  // Drops the recording without saving it. Does nothing when idle.
  void Discard() { recording_.reset(); }

  [[nodiscard]] bool IsRecording() const { return recording_.has_value(); }
  [[nodiscard]] std::size_t FrameCount() const { return recording_ ? recording_->buttons.size() : 0; }

private:
  std::optional<Recording> recording_;
};

} // namespace nes
