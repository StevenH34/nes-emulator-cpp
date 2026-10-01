#pragma once

namespace nes_frontend {

// Audio buffer pacing: keep roughly 2 frames of audio queued in the SDL
// audio stream, nudging playback speed up/down to correct drift.
inline constexpr int kBytesPerSample = sizeof(float);
inline constexpr int kSamplesPerFrameEstimate = 735; // ~44100 / 60
inline constexpr int kTargetQueuedBytes = 2 * kSamplesPerFrameEstimate * kBytesPerSample;
inline constexpr int kQueuedMarginBytes = kSamplesPerFrameEstimate * kBytesPerSample;
inline constexpr float kFastPlaybackRatio = 1.005f;
inline constexpr float kSlowPlaybackRatio = 0.995f;
inline constexpr float kNormalPlaybackRatio = 1.0f;

} // namespace nes_frontend
