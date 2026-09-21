#include "NesApp.h"

#include <stdexcept>

namespace nes_app {

namespace {
// NTSC NES PPU runs at ~60.0988 Hz.
constexpr double kFrameTimeMs = 1000.0 / 60.0988;

// Audio buffer pacing: keep roughly 2 frames of audio queued in the SDL
// audio stream, nudging playback speed up/down to correct drift.
constexpr int kBytesPerSample = sizeof(float);
constexpr int kSamplesPerFrameEstimate = 735; // ~44100 / 60
constexpr int kTargetQueuedBytes = 2 * kSamplesPerFrameEstimate * kBytesPerSample;
constexpr int kQueuedMarginBytes = kSamplesPerFrameEstimate * kBytesPerSample;
constexpr float kFastPlaybackRatio = 1.005f;
constexpr float kSlowPlaybackRatio = 0.995f;
constexpr float kNormalPlaybackRatio = 1.0f;
} // namespace

NesApp::SdlLifetime::SdlLifetime() {
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
    throw std::runtime_error("SDL_Init failed: " + std::string(SDL_GetError()));
  }
}

NesApp::SdlLifetime::~SdlLifetime() { SDL_Quit(); }

NesApp::NesApp(const std::string& rom_path)
    : emulator_(rom_path), window_("NES Emulator", WINDOW_WIDTH, WINDOW_HEIGHT) {
  try {
    renderer_ = SDL_CreateRenderer(static_cast<SDL_Window*>(window_), nullptr);
    if (renderer_ == nullptr) {
      throw std::runtime_error("SDL_CreateRenderer failed: " + std::string(SDL_GetError()));
    }

    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, nes::Ppu::WIDTH,
                                 nes::Ppu::HEIGHT);
    if (texture_ == nullptr) {
      throw std::runtime_error("SDL_CreateTexture failed: " + std::string(SDL_GetError()));
    }

    const SDL_AudioSpec audio_spec{SDL_AUDIO_F32, 1, 44100}; // mono float32 @ 44.1kHz, matching Apu's SAMPLE_RATE
    audio_stream_ = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audio_spec, nullptr, nullptr);
    if (audio_stream_ == nullptr) {
      throw std::runtime_error("SDL_OpenAudioDeviceStream failed: " + std::string(SDL_GetError()));
    }
    SDL_ResumeAudioStreamDevice(audio_stream_);
  } catch (...) {
    Cleanup();
    throw;
  }
}

NesApp::~NesApp() { Cleanup(); }

void NesApp::Cleanup() {
  if (audio_stream_ != nullptr) {
    SDL_DestroyAudioStream(audio_stream_); // also closes the device it's bound to
    audio_stream_ = nullptr;
  }
  if (texture_ != nullptr) {
    SDL_DestroyTexture(texture_);
    texture_ = nullptr;
  }
  if (renderer_ != nullptr) {
    SDL_DestroyRenderer(renderer_);
    renderer_ = nullptr;
  }
}

void NesApp::Run() {
  while (running_) {
    const uint64_t frame_start = SDL_GetTicks();

    HandleEvents();

    const auto& frame_buffer = emulator_.RunFrame();

    const auto samples = emulator_.GetApu().DrainSamples();
    SDL_PutAudioStreamData(audio_stream_, samples.data(), static_cast<int>(samples.size() * sizeof(float)));

    const int queued = SDL_GetAudioStreamQueued(audio_stream_);
    if (queued > kTargetQueuedBytes + kQueuedMarginBytes) {
      SDL_SetAudioStreamFrequencyRatio(audio_stream_, kFastPlaybackRatio);
    } else if (queued < kTargetQueuedBytes - kQueuedMarginBytes) {
      SDL_SetAudioStreamFrequencyRatio(audio_stream_, kSlowPlaybackRatio);
    } else {
      SDL_SetAudioStreamFrequencyRatio(audio_stream_, kNormalPlaybackRatio);
    }

    SDL_UpdateTexture(texture_, nullptr, frame_buffer.data(), nes::Ppu::WIDTH * 4);
    SDL_RenderClear(renderer_);
    SDL_RenderTexture(renderer_, texture_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);

    // ~60 FPS (NTSC)
    if (const double elapsed_time = static_cast<double>(SDL_GetTicks() - frame_start); elapsed_time < kFrameTimeMs) {
      SDL_Delay(static_cast<uint32_t>(kFrameTimeMs - elapsed_time));
    }
  }
}

const std::unordered_map<SDL_Scancode, uint8_t>& NesApp::KeyMap() {
  static const std::unordered_map<SDL_Scancode, uint8_t> key_map = {{
      {SDL_SCANCODE_Z, nes::Controller::BUTTON_A},
      {SDL_SCANCODE_X, nes::Controller::BUTTON_B},
      {SDL_SCANCODE_LSHIFT, nes::Controller::BUTTON_SELECT},
      {SDL_SCANCODE_RSHIFT, nes::Controller::BUTTON_SELECT},
      {SDL_SCANCODE_RETURN, nes::Controller::BUTTON_START},
      {SDL_SCANCODE_UP, nes::Controller::BUTTON_UP},
      {SDL_SCANCODE_DOWN, nes::Controller::BUTTON_DOWN},
      {SDL_SCANCODE_LEFT, nes::Controller::BUTTON_LEFT},
      {SDL_SCANCODE_RIGHT, nes::Controller::BUTTON_RIGHT},
  }};
  return key_map;
}

void NesApp::HandleEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT ||
        (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_ESCAPE)) {
      running_ = false;
    }
    if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
      const auto& key_map = KeyMap();
      if (auto it = key_map.find(event.key.scancode); it != key_map.end()) {
        if (event.type == SDL_EVENT_KEY_DOWN) {
          emulator_.GetBus().GetController1().Press(it->second);
        } else {
          emulator_.GetBus().GetController1().Release(it->second);
        }
      }
    }
  }
}

} // namespace nes_app
