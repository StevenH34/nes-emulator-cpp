#pragma once

#include "Emulator.h"
#include "recording/InputRecording.h"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <format>
#include <functional>
#include <optional>
#include <string>
#include <utility>

namespace nes_frontend {

// Local wall-clock time used to name a recording file.
struct RecordingTime {
  int year{};
  int month{};
  int day{};
  int hour{};
  int minute{};
  int second{};
  int millisecond{};
};

// <ROM folder>/recordings/<ROM name>-YYYYMMDD-HHMMSS-mmm.nesdemo. Doesn't touch the disk.
// Milliseconds keep two clips saved in the same second from overwriting each other.
inline std::string RecordingPath(const std::string& rom_path, const RecordingTime& time) {
  const std::filesystem::path rom(rom_path);
  const std::string name =
    std::format("{}-{:04}{:02}{:02}-{:02}{:02}{:02}-{:03}.nesdemo", rom.stem().string(), time.year, time.month,
                time.day, time.hour, time.minute, time.second, time.millisecond);
  return (rom.parent_path() / "recordings" / name).string();
}

/* The app's F10 recording logic, kept free of SDL so it can be tested.
 * Recording -> Stop() saves the clip. If saving fails, the clip is kept and
 * Paused: no more frames are added, so a retry saves it exactly as it was when
 * the user stopped. StopBeforeJump() is for load state, ROM change and quit:
 * the game state is about to jump, so a clip that can't be saved is discarded.
 */
class RecordingSession {
public:
  enum class State { Idle, Recording, Paused };

  // new_path is called only when saving; it may throw, which counts as a failed save.
  explicit RecordingSession(std::function<std::string()> new_path) : new_path_(std::move(new_path)) {}
  // Non-copyable and non-movable: new_path usually captures its owner's this, and a
  // copy would duplicate the in-progress clip.
  RecordingSession(const RecordingSession&) = delete;
  RecordingSession& operator=(const RecordingSession&) = delete;
  RecordingSession(RecordingSession&&) = delete;
  RecordingSession& operator=(RecordingSession&&) = delete;

  [[nodiscard]] State GetState() const {
    if (!recorder_.IsRecording()) {
      return State::Idle;
    }
    return paused_ ? State::Paused : State::Recording;
  }
  [[nodiscard]] std::size_t FrameCount() const { return recorder_.FrameCount(); }

  // Throws if already recording or the start state can't be taken.
  void Start(nes::Emulator& emulator) { recorder_.Start(emulator); }

  // Call after each frame that ran successfully.
  void OnFrameCompleted(const uint8_t buttons) {
    if (GetState() == State::Recording) {
      recorder_.RecordFrame(buttons);
    }
  }

  // Saves and stops. On failure keeps the clip, pauses, and returns the error.
  std::optional<std::string> Stop() {
    auto error = Save();
    paused_ = error.has_value();
    return error;
  }

  // Saves and stops. On failure discards the clip and returns the error.
  std::optional<std::string> StopBeforeJump() {
    auto error = Save();
    if (error) {
      recorder_.Discard();
    }
    paused_ = false;
    return error;
  }

private:
  // Returns the error message if saving failed; does nothing when idle.
  std::optional<std::string> Save() {
    if (!recorder_.IsRecording()) {
      return std::nullopt;
    }
    try {
      recorder_.Finish(new_path_());
      return std::nullopt;
    } catch (const std::exception& e) {
      return e.what();
    } catch (...) {
      return "unknown error";
    }
  }

  std::function<std::string()> new_path_;
  nes::Recorder recorder_;
  bool paused_{false};
};

} // namespace nes_frontend
