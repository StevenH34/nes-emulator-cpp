#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace nes::ai {

inline constexpr std::size_t OBS_SIZE = 84;
using Observation = std::array<uint8_t, OBS_SIZE * OBS_SIZE>;

/* Turns one NES frame into a small grayscale image the NN can process.
 * Converts a 256x240 RGBA frame (e.g. from Ppu::GetFrameBuffer()) to an 84x84 grayscale observation.
 * 245,760 bytes of input -> 7,056 bytes of output.
 * The observation is row-major: pixel (x, y) is at index y * OBS_SIZE + x.
 * Throws std::invalid_argument if rgba is not exactly 256 * 240 * 4 bytes.
 */
Observation ToObservation(std::span<const uint8_t> rgba);

} // namespace nes::ai