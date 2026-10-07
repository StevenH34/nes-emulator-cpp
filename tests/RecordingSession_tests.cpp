#include "doctest.h"

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include "../src/core/Emulator.h"
#include "../src/core/recording/InputRecording.h"
#include "../src/frontend/RecordingSession.h"
#include "TestRom.h"

using nes_frontend::RecordingSession;
using State = RecordingSession::State;

namespace {

// An emulator plus a session whose save path the test controls: good_path() saves
// into a temp directory, bad_path() points into a folder that doesn't exist.
struct SessionFixture {
  nes_test::TempRomFile rom{nes_test::MakeMinimalRom()};
  nes::Emulator emulator{rom.path()};
  nes_test::TempDir dir{"nes_test_session"};
  std::string next_path;
  int paths_requested = 0;
  RecordingSession session{[this] {
    ++paths_requested;
    return next_path;
  }};

  [[nodiscard]] std::string good_path() const { return (dir.path() / "clip.nesdemo").string(); }
  [[nodiscard]] std::string bad_path() const { return (dir.path() / "missing" / "clip.nesdemo").string(); }

  void Record(const std::vector<uint8_t>& buttons) {
    for (const uint8_t mask : buttons) {
      session.OnFrameCompleted(mask);
    }
  }
};

} // namespace

// --- RecordingSession ---

TEST_CASE("RecordingSession starts idle and ignores frames and stops while idle") {
  SessionFixture f;

  CHECK(f.session.GetState() == State::Idle);
  f.session.OnFrameCompleted(0x80);
  CHECK(f.session.FrameCount() == 0);
  CHECK_FALSE(f.session.Stop().has_value());
  CHECK_FALSE(f.session.StopBeforeJump().has_value());
  CHECK(f.paths_requested == 0); // nothing to save, so no path is needed
  CHECK(f.session.GetState() == State::Idle);
}

TEST_CASE("RecordingSession::Stop saves exactly the recorded frames and returns to idle") {
  SessionFixture f;
  f.session.Start(f.emulator);
  CHECK(f.session.GetState() == State::Recording);
  f.Record({0x80, 0x81, 0x01});
  CHECK(f.session.FrameCount() == 3);

  f.next_path = f.good_path();
  CHECK_FALSE(f.session.Stop().has_value());

  CHECK(f.session.GetState() == State::Idle);
  CHECK(f.paths_requested == 1);
  const auto recording = nes::LoadRecording(f.good_path());
  CHECK(recording.buttons == std::vector<uint8_t>{0x80, 0x81, 0x01});
  CHECK(recording.rom_checksum == f.emulator.GetCartridge().GetRomChecksum());
}

TEST_CASE("RecordingSession::Start throws while a recording is active or paused") {
  SessionFixture f;
  f.session.Start(f.emulator);
  CHECK_THROWS_AS(f.session.Start(f.emulator), std::logic_error);

  f.next_path = f.bad_path();
  REQUIRE(f.session.Stop().has_value());
  REQUIRE(f.session.GetState() == State::Paused);
  CHECK_THROWS_AS(f.session.Start(f.emulator), std::logic_error);
}

TEST_CASE("RecordingSession::Stop pauses on a failed save, and a retry saves the clip as it was when stopped") {
  SessionFixture f;
  f.session.Start(f.emulator);
  f.Record({0x80, 0x01});

  f.next_path = f.bad_path();
  const auto error = f.session.Stop();
  REQUIRE(error.has_value());
  CHECK_FALSE(error->empty());
  CHECK(f.session.GetState() == State::Paused);

  // Play after the failed stop must not end up in the clip
  f.Record({0x40, 0x40, 0x40});
  CHECK(f.session.FrameCount() == 2);

  f.next_path = f.good_path();
  CHECK_FALSE(f.session.Stop().has_value());
  CHECK(f.session.GetState() == State::Idle);
  CHECK(nes::LoadRecording(f.good_path()).buttons == std::vector<uint8_t>{0x80, 0x01});
}

TEST_CASE("RecordingSession::Stop treats a path provider that throws as a failed save") {
  SessionFixture f;
  RecordingSession session{[]() -> std::string { throw std::runtime_error("no clock"); }};
  session.Start(f.emulator);
  session.OnFrameCompleted(0x80);

  const auto error = session.Stop();
  REQUIRE(error.has_value());
  CHECK(*error == "no clock");
  CHECK(session.GetState() == State::Paused);
  CHECK(session.FrameCount() == 1);
}

TEST_CASE("RecordingSession treats a non-standard exception from the path provider as a failed save") {
  SessionFixture f;
  RecordingSession session{[]() -> std::string { throw 42; }};
  session.Start(f.emulator);
  session.OnFrameCompleted(0x80);

  const auto stop_error = session.Stop();
  REQUIRE(stop_error.has_value());
  CHECK(*stop_error == "unknown error");
  CHECK(session.GetState() == State::Paused);

  const auto jump_error = session.StopBeforeJump();
  REQUIRE(jump_error.has_value());
  CHECK(*jump_error == "unknown error");
  CHECK(session.GetState() == State::Idle);
  CHECK(session.FrameCount() == 0);
}

TEST_CASE("RecordingSession::StopBeforeJump saves the clip and returns to idle") {
  SessionFixture f;
  f.session.Start(f.emulator);
  f.Record({0x08, 0x08});

  f.next_path = f.good_path();
  CHECK_FALSE(f.session.StopBeforeJump().has_value());
  CHECK(f.session.GetState() == State::Idle);
  CHECK(nes::LoadRecording(f.good_path()).buttons == std::vector<uint8_t>{0x08, 0x08});
}

TEST_CASE("RecordingSession::StopBeforeJump discards the clip when saving fails") {
  SessionFixture f;
  f.session.Start(f.emulator);
  f.Record({0x08, 0x08});

  f.next_path = f.bad_path();
  CHECK(f.session.StopBeforeJump().has_value());
  CHECK(f.session.GetState() == State::Idle);
  CHECK(f.session.FrameCount() == 0);
  CHECK(nes_test::EntryNames(f.dir.path()).empty());

  // A new recording can start afterwards
  f.session.Start(f.emulator);
  CHECK(f.session.GetState() == State::Recording);
}

TEST_CASE("RecordingSession::StopBeforeJump while paused retries the save once, then discards") {
  SUBCASE("the retry succeeds") {
    SessionFixture f;
    f.session.Start(f.emulator);
    f.Record({0x80});
    f.next_path = f.bad_path();
    REQUIRE(f.session.Stop().has_value());

    f.next_path = f.good_path();
    CHECK_FALSE(f.session.StopBeforeJump().has_value());
    CHECK(f.session.GetState() == State::Idle);
    CHECK(nes::LoadRecording(f.good_path()).buttons == std::vector<uint8_t>{0x80});
  }
  SUBCASE("the retry fails") {
    SessionFixture f;
    f.session.Start(f.emulator);
    f.Record({0x80});
    f.next_path = f.bad_path();
    REQUIRE(f.session.Stop().has_value());

    CHECK(f.session.StopBeforeJump().has_value());
    CHECK(f.session.GetState() == State::Idle);
    CHECK(f.session.FrameCount() == 0);
    CHECK(f.paths_requested == 2);
  }
}

// --- RecordingPath ---

TEST_CASE("RecordingPath puts the clip in a recordings folder next to the ROM, named by zero-padded local time") {
  const std::filesystem::path rom = std::filesystem::path("roms") / "smb.nes";
  const std::string path = nes_frontend::RecordingPath(rom.string(), {2026, 3, 4, 5, 6, 7, 8});

  const std::filesystem::path expected =
    std::filesystem::path("roms") / "recordings" / "smb-20260304-050607-008.nesdemo";
  CHECK(path == expected.string());
}

TEST_CASE("RecordingPath keeps two-digit and three-digit fields unchanged") {
  const std::string path = nes_frontend::RecordingPath("smb.nes", {2026, 12, 31, 23, 59, 58, 999});
  CHECK(std::filesystem::path(path).filename() == "smb-20261231-235958-999.nesdemo");
  CHECK(std::filesystem::path(path).parent_path() == "recordings");
}

TEST_CASE("RecordingPath uses only the ROM's name without its extension") {
  const std::string path = nes_frontend::RecordingPath("Super Mario Bros. (World).nes", {2026, 1, 1, 0, 0, 0, 0});
  CHECK(std::filesystem::path(path).filename() == "Super Mario Bros. (World)-20260101-000000-000.nesdemo");
}
