#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace nes::ai {

inline constexpr std::size_t OBS_SIZE = 84;
using Observation = std::array<uint8_t, OBS_SIZE * OBS_SIZE>;

// Gets frame from Ppu::FrameBuffer(), then converts 256x240 RGBA frame to 84x84 grayscale observation.
Observation ToObservation(std::span<const uint8_t> rgba);

} // namespace nes::ai