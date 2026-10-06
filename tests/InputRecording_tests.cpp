#include "doctest.h"

#include <array>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include "../src/core/Cartridge.h"
#include "../src/core/Emulator.h"
#include "../src/core/recording/InputRecording.h"
#include "../src/core/save_state/StateReader.h"
#include "TestRom.h"

namespace {

// Byte offset of the start-state size field: "NESR" (4) + version (2) + ROM checksum (4).
constexpr std::size_t kStateSizeOffset = 10;

nes::Recording MakeSampleRecording() {
  return nes::Recording{0xDEADBEEF, {0x10, 0x20, 0x30, 0x40, 0x50}, {0x00, 0x80, 0x81, 0x01}};
}

} // namespace

// --- RecordingToBytes / RecordingFromBytes ---

TEST_CASE("RecordingToBytes writes the NESR magic, version 1, checksum, start state, frame count and buttons") {
  const auto recording = MakeSampleRecording();
  const auto bytes = nes::RecordingToBytes(recording);
  nes::StateReader reader(bytes);

  std::array<uint8_t, 4> magic{};
  reader.ReadBytes(magic);
  CHECK(magic == std::array<uint8_t, 4>{'N', 'E', 'S', 'R'});
  CHECK(reader.ReadU16() == 1);
  CHECK(reader.ReadU32() == 0xDEADBEEF);
  REQUIRE(reader.ReadU32() == recording.start_state.size());
  std::vector<uint8_t> start_state(recording.start_state.size());
  reader.ReadBytes(start_state);
  CHECK(start_state == recording.start_state);
  REQUIRE(reader.ReadU32() == recording.buttons.size());
  std::vector<uint8_t> buttons(recording.buttons.size());
  reader.ReadBytes(buttons);
  CHECK(buttons == recording.buttons);
  CHECK(reader.BytesRemaining() == 0);
}

TEST_CASE("RecordingFromBytes round-trips RecordingToBytes") {
  const auto original = MakeSampleRecording();
  const auto restored = nes::RecordingFromBytes(nes::RecordingToBytes(original));

  CHECK(restored.rom_checksum == original.rom_checksum);
  CHECK(restored.start_state == original.start_state);
  CHECK(restored.buttons == original.buttons);
}

TEST_CASE("RecordingFromBytes round-trips a recording with no frames") {
  const nes::Recording original{1, {0xAA}, {}};
  const auto restored = nes::RecordingFromBytes(nes::RecordingToBytes(original));

  CHECK(restored.start_state == original.start_state);
  CHECK(restored.buttons.empty());
}

TEST_CASE("RecordingFromBytes throws when the magic number is wrong") {
  auto bytes = nes::RecordingToBytes(MakeSampleRecording());
  bytes[0] = 'X';
  CHECK_THROWS_AS(static_cast<void>(nes::RecordingFromBytes(bytes)), std::runtime_error);
}

TEST_CASE("RecordingFromBytes throws when the version is unsupported") {
  auto bytes = nes::RecordingToBytes(MakeSampleRecording());
  bytes[4] = 0x02; // version low byte: 1 -> 2
  CHECK_THROWS_AS(static_cast<void>(nes::RecordingFromBytes(bytes)), std::runtime_error);
}

TEST_CASE("RecordingFromBytes throws when the start state size is larger than the data") {
  auto bytes = nes::RecordingToBytes(MakeSampleRecording());
  for (std::size_t i = 0; i < 4; ++i) {
    bytes[kStateSizeOffset + i] = 0xFF; // claims a ~4 GB start state
  }
  CHECK_THROWS_AS(static_cast<void>(nes::RecordingFromBytes(bytes)), std::runtime_error);
}

TEST_CASE("RecordingFromBytes throws when the button data is shorter or longer than the frame count") {
  auto bytes = nes::RecordingToBytes(MakeSampleRecording());

  SUBCASE("one button byte missing") { bytes.pop_back(); }
  SUBCASE("one extra byte at the end") { bytes.push_back(0x00); }

  CHECK_THROWS_AS(static_cast<void>(nes::RecordingFromBytes(bytes)), std::runtime_error);
}

// Under local Windows Clang ASan this aborts (throw from inside a catch); see the ASan exclusion list.
TEST_CASE("RecordingFromBytes throws std::runtime_error, not std::out_of_range, when the header is truncated") {
  const auto full = nes::RecordingToBytes(MakeSampleRecording());

  SUBCASE("empty") {
    CHECK_THROWS_AS(static_cast<void>(nes::RecordingFromBytes(std::vector<uint8_t>{})), std::runtime_error);
  }
  SUBCASE("cut inside the version field") {
    const std::vector<uint8_t> bytes(full.begin(), full.begin() + 5);
    CHECK_THROWS_AS(static_cast<void>(nes::RecordingFromBytes(bytes)), std::runtime_error);
  }
}

// --- SaveRecording / LoadRecording ---

TEST_CASE("SaveRecording then LoadRecording round-trips and leaves no temp file behind") {
  const nes_test::TempDir dir("nes_test_recording");
  const std::string path = (dir.path() / "clip.nesdemo").string();
  const auto original = MakeSampleRecording();

  nes::SaveRecording(original, path);
  const auto restored = nes::LoadRecording(path);

  CHECK(restored.rom_checksum == original.rom_checksum);
  CHECK(restored.start_state == original.start_state);
  CHECK(restored.buttons == original.buttons);
  CHECK(nes_test::EntryNames(dir.path()) == std::vector<std::string>{"clip.nesdemo"});
}

TEST_CASE("LoadRecording throws when the file does not exist") {
  CHECK_THROWS_AS(static_cast<void>(nes::LoadRecording("/no/such/file/definitely_missing.nesdemo")),
                  std::runtime_error);
}

// --- Recorder ---

TEST_CASE("Recorder captures the emulator's start state and checksum, then each recorded frame") {
  const nes_test::TempRomFile rom(nes_test::MakeMinimalRom());
  nes::Emulator emulator(rom.path());
  const nes_test::TempDir dir("nes_test_recording");
  const std::string path = (dir.path() / "clip.nesdemo").string();
  const auto start_state = emulator.SaveStateToBytes();

  nes::Recorder recorder;
  CHECK_FALSE(recorder.IsRecording());
  recorder.Start(emulator);
  CHECK(recorder.IsRecording());
  recorder.RecordFrame(0x80);
  recorder.RecordFrame(0x01);
  recorder.RecordFrame(0x81);
  CHECK(recorder.FrameCount() == 3);
  recorder.Finish(path);

  CHECK_FALSE(recorder.IsRecording());
  CHECK(recorder.FrameCount() == 0);
  const auto recording = nes::LoadRecording(path);
  CHECK(recording.rom_checksum == emulator.GetCartridge().GetRomChecksum());
  CHECK(recording.start_state == start_state);
  CHECK(recording.buttons == std::vector<uint8_t>{0x80, 0x01, 0x81});
}

TEST_CASE("Recorder throws std::logic_error when called in the wrong state") {
  const nes_test::TempRomFile rom(nes_test::MakeMinimalRom());
  nes::Emulator emulator(rom.path());
  nes::Recorder recorder;

  CHECK_THROWS_AS(recorder.RecordFrame(0x00), std::logic_error);
  CHECK_THROWS_AS(recorder.Finish("unused.nesdemo"), std::logic_error);
  recorder.Start(emulator);
  CHECK_THROWS_AS(recorder.Start(emulator), std::logic_error);
}

TEST_CASE("Recorder::Discard drops the recording, and does nothing when idle") {
  const nes_test::TempRomFile rom(nes_test::MakeMinimalRom());
  nes::Emulator emulator(rom.path());
  nes::Recorder recorder;

  recorder.Discard();
  CHECK_FALSE(recorder.IsRecording());

  recorder.Start(emulator);
  recorder.RecordFrame(0x80);
  recorder.Discard();
  CHECK_FALSE(recorder.IsRecording());
  CHECK(recorder.FrameCount() == 0);
  recorder.Start(emulator); // can record again afterwards
  CHECK(recorder.IsRecording());
}

TEST_CASE("Recorder::Finish keeps the recording when the file can't be written") {
  const nes_test::TempRomFile rom(nes_test::MakeMinimalRom());
  nes::Emulator emulator(rom.path());
  const nes_test::TempDir dir("nes_test_recording");
  nes::Recorder recorder;

  recorder.Start(emulator);
  recorder.RecordFrame(0x80);
  recorder.RecordFrame(0x01);
  CHECK_THROWS_AS(recorder.Finish((dir.path() / "missing" / "clip.nesdemo").string()), std::runtime_error);
  CHECK(nes_test::EntryNames(dir.path()).empty());
  CHECK(recorder.IsRecording());
  CHECK(recorder.FrameCount() == 2);

  // A retry to a valid path saves every frame
  const std::string path = (dir.path() / "clip.nesdemo").string();
  recorder.Finish(path);
  CHECK(nes::LoadRecording(path).buttons == std::vector<uint8_t>{0x80, 0x01});
}
