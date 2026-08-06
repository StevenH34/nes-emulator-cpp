#include "NesApp.h"

#include <stdexcept>

namespace nes_app {

namespace {
// NTSC NES PPU runs at ~60.0988 Hz.
constexpr double kFrameTimeMs = 1000.0 / 60.0988;
} // namespace

NesApp::SdlLifetime::SdlLifetime() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    throw std::runtime_error("SDL_Init failed: " + std::string(SDL_GetError()));
  }
}

NesApp::SdlLifetime::~SdlLifetime() { SDL_Quit(); }

NesApp::NesApp(const std::string& rom_path)
    : emulator_(rom_path), window_("NES Emulator", WINDOW_WIDTH, WINDOW_HEIGHT) {
  try {
    renderer_ = SDL_CreateRenderer(static_cast<SDL_Window *>(window_), nullptr);
    if (renderer_ == nullptr) {
      throw std::runtime_error("SDL_CreateRenderer failed: " + std::string(SDL_GetError()));
    }

    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, nes::Ppu::WIDTH,
                                 nes::Ppu::HEIGHT);
    if (texture_ == nullptr) {
      throw std::runtime_error("SDL_CreateTexture failed: " + std::string(SDL_GetError()));
    }

    // TODO: Add audio
  } catch (...) {
    Cleanup();
    throw;
  }
}

NesApp::~NesApp() { Cleanup(); }

void NesApp::Cleanup() {
  // TODO: Destroy audio
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

    // TODO: Add audio

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
