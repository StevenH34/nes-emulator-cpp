#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Emulator.h"
#include "Window.h"

namespace nes_app {

class NesApp {
public:
  explicit NesApp(const std::optional<std::string>& rom_path);
  ~NesApp();
  // Non-copyable
  NesApp(const NesApp&) = delete;
  NesApp& operator=(const NesApp&) = delete;

  // Picture scale at 100% display scaling; multiplied by the display's
  // content scale at startup, then reduced until the window fits the screen.
  static constexpr int DEFAULT_SCALE = 3;
  // Menu bar height with ImGui's default font and style at 100% display
  // scaling; corrected on the first frame if the real height differs.
  static constexpr float DEFAULT_MENU_BAR_HEIGHT = 19.0f;

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
  void DrawMenuBar();
  void Render(const std::vector<uint8_t>* frame_buffer);
  void Cleanup();
  // Throws if the ROM can't be loaded; the current emulator is left untouched.
  void LoadRom(const std::string& rom_path);
  // Like LoadRom, but shows an error box instead of throwing.
  void OpenRom(const std::string& rom_path);
  void ShowOpenRomDialog();
  void ApplyPendingRom();
  void Reset();
  void SaveState();
  void LoadState();
  void ShowError(const std::string& message) const;

  static void SDLCALL OnOpenRomDialogResult(void* userdata, const char* const* filelist, int filter);

  [[nodiscard]] int WindowWidth() const;
  [[nodiscard]] int WindowHeight() const;

  std::unique_ptr<nes::Emulator> emulator_;
  bool running_{true};
  SdlLifetime sdl_;
  // These size the window, so they must be declared after sdl_ (they query
  // the display) and before window_, in this order.
  float content_scale_;
  float menu_bar_height_;
  int scale_;
  nes::Window window_;
  SDL_Renderer* renderer_{nullptr};
  SDL_Texture* texture_{nullptr};
  SDL_AudioStream* audio_stream_{nullptr};
  std::string save_state_path_;
  std::string rom_path_;
  // File dialog callback may run on a separate thread, so we need to synchronize access to pending_rom_path_.
  std::mutex pending_rom_mutex_;
  std::optional<std::string> pending_rom_path_;
};

} // namespace nes_app