#pragma once

#include <string>
#include <unordered_map>
#include <SDL3/SDL.h>

#include "Emulator.h"
#include "Window.h"

namespace nes_app {

class NesApp {
public:
  explicit NesApp(const std::string& rom_path);
  ~NesApp();
  // Non-copyable
  NesApp(const NesApp&) = delete;
  NesApp& operator=(const NesApp&) = delete;

  static constexpr int SCALE = 3;
  static constexpr int WINDOW_WIDTH = nes::Ppu::WIDTH * SCALE; // 768
  static constexpr int WINDOW_HEIGHT = nes::Ppu::HEIGHT * SCALE; // 720

  void Run();

  // Key bindings
  static const std::unordered_map<SDL_Scancode, uint8_t>& KeyMap();

private:
  // Owns the SDL library lifetime; must outlive window_ and be
  // constructed before it, since window creation requires SDL_Init.
  struct SdlLifetime {
    SdlLifetime();
    ~SdlLifetime();
  };

  void HandleEvents();
  void Cleanup();
  void SaveState();
  void LoadState();

  nes::Emulator emulator_;
  bool running_{true};
  SdlLifetime sdl_;
  nes::Window window_;
  SDL_Renderer* renderer_{nullptr};
  SDL_Texture* texture_{nullptr};
  SDL_AudioStream* audio_stream_{nullptr};
  std::string save_state_path_;
};

} // namespace nes_app
