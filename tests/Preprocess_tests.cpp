#include "doctest.h"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "../src/core/ai/Preprocess.h"
#include "../src/core/ppu/Ppu.h"

namespace {

constexpr std::size_t FRAME_WIDTH = nes::Ppu::WIDTH;
constexpr std::size_t FRAME_HEIGHT = nes::Ppu::HEIGHT;

// Builds a 256x240 RGBA frame filled with a single opaque color.
std::vector<uint8_t> MakeFrame(const uint8_t r, const uint8_t g, const uint8_t b) {
  std::vector<uint8_t> frame(FRAME_WIDTH * FRAME_HEIGHT * 4);
  for (std::size_t i = 0; i < frame.size(); i += 4) {
    frame[i] = r;
    frame[i + 1] = g;
    frame[i + 2] = b;
    frame[i + 3] = 0xFF;
  }
  return frame;
}

void SetPixel(std::vector<uint8_t>& frame, const std::size_t x, const std::size_t y, const uint8_t value) {
  const std::size_t offset = (y * FRAME_WIDTH + x) * 4;
  frame[offset] = value;
  frame[offset + 1] = value;
  frame[offset + 2] = value;
}

uint8_t ObsAt(const nes::ai::Observation& observation, const std::size_t x, const std::size_t y) {
  return observation[y * nes::ai::OBS_SIZE + x];
}

} // namespace

TEST_CASE("ToObservation throws std::invalid_argument when the frame is the wrong size") {
  const std::vector<uint8_t> too_small(FRAME_WIDTH * FRAME_HEIGHT * 3);
  CHECK_THROWS_AS(nes::ai::ToObservation(too_small), std::invalid_argument);
  CHECK_THROWS_AS(nes::ai::ToObservation({}), std::invalid_argument);
}

TEST_CASE("ToObservation maps an all-black frame to 0 and an all-white frame to 255") {
  const auto black = nes::ai::ToObservation(MakeFrame(0x00, 0x00, 0x00));
  const auto white = nes::ai::ToObservation(MakeFrame(0xFF, 0xFF, 0xFF));

  for (std::size_t i = 0; i < black.size(); ++i) {
    REQUIRE(black[i] == 0);
    REQUIRE(white[i] == 255);
  }
}

TEST_CASE("ToObservation applies the luma weights in R, G, B order") {
  // (77 * 255) >> 8 == 76, (150 * 255) >> 8 == 149, (29 * 255) >> 8 == 28
  const auto red = nes::ai::ToObservation(MakeFrame(0xFF, 0x00, 0x00));
  const auto green = nes::ai::ToObservation(MakeFrame(0x00, 0xFF, 0x00));
  const auto blue = nes::ai::ToObservation(MakeFrame(0x00, 0x00, 0xFF));

  for (std::size_t i = 0; i < red.size(); ++i) {
    REQUIRE(red[i] == 76);
    REQUIRE(green[i] == 149);
    REQUIRE(blue[i] == 28);
  }
}

TEST_CASE("ToObservation averages a box that straddles a white/black edge") {
  // Output column 42 covers source columns 128..130. Making columns 0..129
  // white leaves 2 of its 3 columns white: round(2 * 255 / 3) == 170.
  auto frame = MakeFrame(0x00, 0x00, 0x00);
  for (std::size_t y = 0; y < FRAME_HEIGHT; ++y) {
    for (std::size_t x = 0; x < 130; ++x) {
      SetPixel(frame, x, y, 0xFF);
    }
  }

  const auto observation = nes::ai::ToObservation(frame);

  for (std::size_t y = 0; y < nes::ai::OBS_SIZE; ++y) {
    REQUIRE(ObsAt(observation, 0, y) == 255);
    REQUIRE(ObsAt(observation, 41, y) == 255);
    REQUIRE(ObsAt(observation, 42, y) == 170);
    REQUIRE(ObsAt(observation, 43, y) == 0);
    REQUIRE(ObsAt(observation, 83, y) == 0);
  }
}

TEST_CASE("ToObservation puts a single source pixel into exactly one 3x2 output box") {
  // Output (0, 0) covers source columns 0..2 and rows 0..1: 6 pixels,
  // so one white pixel averages to round(255 / 6) == 43.
  auto frame = MakeFrame(0x00, 0x00, 0x00);
  SetPixel(frame, 0, 0, 0xFF);

  const auto observation = nes::ai::ToObservation(frame);

  CHECK(ObsAt(observation, 0, 0) == 43);
  for (std::size_t i = 1; i < observation.size(); ++i) {
    REQUIRE(observation[i] == 0);
  }
}
