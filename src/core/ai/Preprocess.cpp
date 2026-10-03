#include "Preprocess.h"

#include "ppu/Ppu.h"

#include <format>
#include <stdexcept>

namespace nes::ai {

namespace {

// For the input frame taken from the PPU
constexpr std::size_t SRC_WIDTH = Ppu::WIDTH; // 256
constexpr std::size_t SRC_HEIGHT = Ppu::HEIGHT; // 240
constexpr std::size_t BYTES_PER_PIXEL = 4; // RGBA

/*
 * Color to brightness conversion.
 * Standard BT.601 weights (0.299, 0.587, 0.114) scaled by 256.
 * White stays 255 and black stays 0.
 *
 * @param rgba The input RGBA pixel data.
 * @param offset The offset to a pixel's R , G, and B byte. A is ignored.
 * @return The luma (brightness) value of the pixel.
 * `>> 8` is used to divide by 256, effectively scaling the weighted sum down to the 0-255 range.
 */
uint32_t Luma(const std::span<const uint8_t> rgba, const std::size_t offset) {
  return (77u * rgba[offset] + 150u * rgba[offset + 1] + 29u * rgba[offset + 2]) >> 8;
}

} // namespace

/*
 * Turing one NES frame into a small greyscale image the NN can process.
 * 256x240 RGBA frame -> ToObservation() -> 84x84 grayscale uint8 observation.
 * 245,760 bytes of input -> 7,056 bytes of output.
 */
Observation ToObservation(const std::span<const uint8_t> rgba) {
  if (constexpr std::size_t expected_size = SRC_WIDTH * SRC_HEIGHT * BYTES_PER_PIXEL; rgba.size() != expected_size) {
    throw std::invalid_argument(
      std::format("ToObservation: Expected {} bytes rgba, got {}", expected_size, rgba.size()));
  }

  // 256x240 image is divided into an 84x84 gird of small boxes.
  Observation observation{};
  for (std::size_t oy = 0; oy < OBS_SIZE; ++oy) {
    // Source rows covered by this output row (2 or 3 rows)
    const std::size_t y_start = oy * SRC_HEIGHT / OBS_SIZE;
    const std::size_t y_end = (oy + 1) * SRC_HEIGHT / OBS_SIZE;

    for (std::size_t ox = 0; ox < OBS_SIZE; ++ox) {
      // Source columns covered by this output column (3 or 4 cols)
      const std::size_t x_start = ox * SRC_WIDTH / OBS_SIZE;
      const std::size_t x_end = (ox + 1) * SRC_WIDTH / OBS_SIZE;

      uint32_t sum = 0;
      for (std::size_t y = y_start; y < y_end; ++y) {
        for (std::size_t x = x_start; x < x_end; ++x) {
          // Add up the brightness for each pixel in the box.
          sum += Luma(rgba, (y * SRC_WIDTH + x) * BYTES_PER_PIXEL);
        }
      }

      // Round to the nearest integer instead of truncating
      const auto count = static_cast<uint32_t>((y_end - y_start) * (x_end - x_start));
      observation[oy * OBS_SIZE + ox] = static_cast<uint8_t>((sum + count / 2) / count);
    }
  }
  return observation;
}

} // namespace nes::ai