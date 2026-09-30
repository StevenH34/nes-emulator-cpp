#pragma once

#include <SDL3/SDL.h>
#include <cstdint>
#include <memory>
#include <span>

#include "Emulator.h"

namespace nes_wasm {

class WasmApp {
public:
  WasmApp();
  ~WasmApp();
  // Non-copyable
  WasmApp(const WasmApp&) = delete;
  WasmApp& operator=(const WasmApp&) = delete;

  static constexpr int SCALE = 3;
  static constexpr int WINDOW_WIDTH = nes::Ppu::WIDTH * SCALE; // 768
  static constexpr int WINDOW_HEIGHT = nes::Ppu::HEIGHT * SCALE; // 720

  // Replaces the running game. Throws (leaving the current game running) if the ROM is invalid.
  void LoadRom(std::span<const uint8_t> rom_bytes);

  // Called once per browser animation frame.
  void Tick();

private:
  // Owns the SDL library lifetime. Must be constructed before any SDL objects.
  struct SdlLifetime {
    SdlLifetime();
    ~SdlLifetime();
  };

  void HandleEvents();
  void Render();
  void Cleanup();

  // No ROM exists at startup, so the emulator is created when JS loads one.
  std::unique_ptr<nes::Emulator> emulator_;
  SdlLifetime sdl_;
  SDL_Window* window_{nullptr};
  SDL_Renderer* renderer_{nullptr};
  SDL_Texture* texture_{nullptr};
  uint64_t last_tick_ns_{0};
  double accumulated_ms_{0.0};
};

// The single app instance owned by main_wasm.cpp, or nullptr before it is created.
WasmApp* GetActiveApp();

} // namespace nes_wasm
