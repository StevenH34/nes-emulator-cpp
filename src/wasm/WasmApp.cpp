#include "WasmApp.h"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace nes_wasm {

namespace {
// NTSC NES PPU runs at ~60.0988 Hz.
constexpr double kFrameTimeMs = 1000.0 / 60.0988;

// requestAnimationFrame fires at the display's refresh rate (60/120/144 Hz...), so emulated
// frames are paced by an accumulator rather than one per callback. The slack lets a slightly
// early callback still run its frame, avoiding 0/2-frame judder on a 60 Hz display.
constexpr double kFrameSlackMs = 2.0;

// After the tab is hidden (rAF paused), skip ahead instead of fast-forwarding through the backlog.
constexpr double kMaxAccumulatedMs = 3 * kFrameTimeMs;

constexpr double kNsPerMs = 1'000'000.0;
} // namespace

WasmApp::SdlLifetime::SdlLifetime() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    throw std::runtime_error("SDL_Init failed: " + std::string(SDL_GetError()));
  }
}

WasmApp::SdlLifetime::~SdlLifetime() { SDL_Quit(); }

WasmApp::WasmApp() {
  try {
    if (!SDL_CreateWindowAndRenderer("NES Emulator", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window_, &renderer_)) {
      throw std::runtime_error("SDL_CreateWindowAndRenderer failed: " + std::string(SDL_GetError()));
    }

    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, nes::Ppu::WIDTH,
                                 nes::Ppu::HEIGHT);
    if (texture_ == nullptr) {
      throw std::runtime_error("SDL_CreateTexture failed: " + std::string(SDL_GetError()));
    }

    SDL_SetTextureScaleMode(texture_, SDL_SCALEMODE_NEAREST);
  } catch (...) {
    Cleanup();
    throw;
  }
  last_tick_ns_ = SDL_GetTicksNS();
}

WasmApp::~WasmApp() { Cleanup(); }

void WasmApp::Cleanup() {
  if (texture_ != nullptr) {
    SDL_DestroyTexture(texture_);
    texture_ = nullptr;
  }
  if (renderer_ != nullptr) {
    SDL_DestroyRenderer(renderer_);
    renderer_ = nullptr;
  }
  if (window_ != nullptr) {
    SDL_DestroyWindow(window_);
    window_ = nullptr;
  }
}

void WasmApp::LoadRom(const std::span<const uint8_t> rom_bytes) {
  auto emulator = std::make_unique<nes::Emulator>(rom_bytes);
  emulator_ = std::move(emulator);
  accumulated_ms_ = 0.0;
}

void WasmApp::Tick() {
  const uint64_t now_ns = SDL_GetTicksNS();
  accumulated_ms_ += static_cast<double>(now_ns - last_tick_ns_) / kNsPerMs;
  accumulated_ms_ = std::min(accumulated_ms_, kMaxAccumulatedMs);
  last_tick_ns_ = now_ns;

  HandleEvents();

  if (emulator_ != nullptr) {
    while (accumulated_ms_ >= kFrameTimeMs - kFrameSlackMs) {
      emulator_->RunFrame();
      accumulated_ms_ -= kFrameTimeMs;
    }
  } else {
    accumulated_ms_ = 0.0;
  }

  Render();
}

void WasmApp::HandleEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
  }
}

void WasmApp::Render() {
  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer_);
  if (emulator_ != nullptr) {
    const auto& frame_buffer = emulator_->GetPpu().GetFrameBuffer();
    SDL_UpdateTexture(texture_, nullptr, frame_buffer.data(), nes::Ppu::WIDTH * 4);
    SDL_RenderTexture(renderer_, texture_, nullptr, nullptr);
  }
  SDL_RenderPresent(renderer_);
}

} // namespace nes_wasm
