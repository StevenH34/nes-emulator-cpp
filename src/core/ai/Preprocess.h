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

/*
 * Grayscale with int for luma.
 * Each source pixel:
 *  Y = (77·R + 150·G + 29·B) >> 8
 *
 * Area average down to 84x84. 256/84 = 3.05 and 240/84 = 2.86.
 * Each output pixel (ox, oy) averages a box of source pixels:
 *  x from ox·256/84 up to (ox+1)·256/84  (integer division → 3 or 4 px wide)
 *  y from oy·240/84 up to (oy+1)·240/84  (→ 2 or 3 px tall)
 *  value = (sum of Y + count/2) / count  (rounded)
 *
 * The loop does about 61k multiply-adds per frame
 *
 */