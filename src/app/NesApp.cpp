#include "NesApp.h"

#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <iostream>
#include <iterator>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

#include "AudioPacing.h"

namespace nes_app {

namespace {
// NTSC NES PPU runs at ~60.0988 Hz.
constexpr double kFrameTimeMs = 1000.0 / 60.0988;
constexpr const char* kWindowTitle = "NES Emulator";
// SDL requires filter to stay valid until dialog callback runs.
constexpr SDL_DialogFileFilter kRomFilters[] = {
  {"NES ROMs", "nes"},
};

// Room left for the title bar and window borders at 100% display scaling;
// their real size isn't known until the window exists.
constexpr float kWindowFrameMargin = 40.0f;

std::string FileName(const std::string& path) {
  const auto pos = path.find_last_of("/\\");
  return pos == std::string::npos ? path : path.substr(pos + 1);
}

// Display scaling of the primary monitor (e.g. 2.25 at 225%), or 1 if unknown.
float PrimaryContentScale() {
  const float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
  return scale > 0.0f ? scale : 1.0f;
}

// DEFAULT_SCALE adjusted for display scaling, reduced until the window fits
// on the primary display.
int StartupScale(const float content_scale, const float menu_bar_height) {
  int scale = std::max(1, static_cast<int>(std::lround(static_cast<float>(NesApp::DEFAULT_SCALE) * content_scale)));

  SDL_Rect usable{};
  if (!SDL_GetDisplayUsableBounds(SDL_GetPrimaryDisplay(), &usable)) {
    return scale;
  }
  const int reserved_height = static_cast<int>(std::ceil(menu_bar_height + kWindowFrameMargin * content_scale));
  while (scale > 1 &&
         (nes::Ppu::WIDTH * scale > usable.w || nes::Ppu::HEIGHT * scale + reserved_height > usable.h)) {
    --scale;
  }
  return scale;
}

using nes_frontend::kFastPlaybackRatio;
using nes_frontend::kNormalPlaybackRatio;
using nes_frontend::kQueuedMarginBytes;
using nes_frontend::kSlowPlaybackRatio;
using nes_frontend::kTargetQueuedBytes;
} // namespace

NesApp::SdlLifetime::SdlLifetime() {
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
    throw std::runtime_error("SDL_Init failed: " + std::string(SDL_GetError()));
  }
}

NesApp::SdlLifetime::~SdlLifetime() { SDL_Quit(); }

NesApp::NesApp(const std::optional<std::string>& rom_path)
    : content_scale_(PrimaryContentScale()),
      menu_bar_height_(std::ceil(DEFAULT_MENU_BAR_HEIGHT * content_scale_)),
      scale_(StartupScale(content_scale_, menu_bar_height_)),
      window_(kWindowTitle, WindowWidth(), WindowHeight(), SDL_WINDOW_RESIZABLE) {
  try {
    // Never let the picture shrink below 1x.
    SDL_SetWindowMinimumSize(static_cast<SDL_Window*>(window_), nes::Ppu::WIDTH,
                             nes::Ppu::HEIGHT + static_cast<int>(menu_bar_height_));

    renderer_ = SDL_CreateRenderer(static_cast<SDL_Window*>(window_), nullptr);
    if (renderer_ == nullptr) {
      throw std::runtime_error("SDL_CreateRenderer failed: " + std::string(SDL_GetError()));
    }

    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, nes::Ppu::WIDTH,
                                 nes::Ppu::HEIGHT);
    if (texture_ == nullptr) {
      throw std::runtime_error("SDL_CreateTexture failed: " + std::string(SDL_GetError()));
    }
    // Keep hard pixel edges when scaling up instead of SDL's default smoothing.
    SDL_SetTextureScaleMode(texture_, SDL_SCALEMODE_NEAREST);

    const SDL_AudioSpec audio_spec{SDL_AUDIO_F32, 1, 44100}; // mono float32 @ 44.1kHz, matching Apu's SAMPLE_RATE
    audio_stream_ = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audio_spec, nullptr, nullptr);
    if (audio_stream_ == nullptr) {
      throw std::runtime_error("SDL_OpenAudioDeviceStream failed: " + std::string(SDL_GetError()));
    }
    SDL_ResumeAudioStreamDevice(audio_stream_);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr; // disable saving to imgui.ini
    // Match the menu bar's size to the display's scaling.
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(content_scale_);
    style.FontScaleDpi = content_scale_;
    if (!ImGui_ImplSDL3_InitForSDLRenderer(static_cast<SDL_Window*>(window_), renderer_)) {
      throw std::runtime_error("ImGui_ImplSDL3_InitForSDLRenderer failed");
    }
    if (!ImGui_ImplSDLRenderer3_Init(renderer_)) {
      throw std::runtime_error("ImGui_ImplSDLRenderer3_Init failed");
    }
    if (rom_path.has_value()) {
      LoadRom(*rom_path);
    }
  } catch (...) {
    Cleanup();
    throw;
  }
}

NesApp::~NesApp() { Cleanup(); }

void NesApp::Cleanup() {
  // Destroy ImGui before destroying the renderer, since it uses the renderer.
  if (ImGui::GetCurrentContext() != nullptr) {
    if (ImGui::GetIO().BackendRendererUserData != nullptr) {
      ImGui_ImplSDLRenderer3_Shutdown();
    }
    if (ImGui::GetIO().BackendPlatformUserData != nullptr) {
      ImGui_ImplSDL3_Shutdown();
    }
    ImGui::DestroyContext();
  }
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
    ApplyPendingRom();

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    DrawMenuBar();

    const std::vector<uint8_t>* frame_buffer = nullptr;
    if (emulator_ != nullptr) {
      frame_buffer = &emulator_->RunFrame();

      const auto samples = emulator_->GetApu().DrainSamples();
      SDL_PutAudioStreamData(audio_stream_, samples.data(), static_cast<int>(samples.size() * sizeof(float)));

      if (const int queued = SDL_GetAudioStreamQueued(audio_stream_);
          queued > kTargetQueuedBytes + kQueuedMarginBytes) {
        SDL_SetAudioStreamFrequencyRatio(audio_stream_, kFastPlaybackRatio);
      } else if (queued < kTargetQueuedBytes - kQueuedMarginBytes) {
        SDL_SetAudioStreamFrequencyRatio(audio_stream_, kSlowPlaybackRatio);
      } else {
        SDL_SetAudioStreamFrequencyRatio(audio_stream_, kNormalPlaybackRatio);
      }
    }

    Render(frame_buffer);

    // ~60 FPS (NTSC)
    if (const double elapsed_time = static_cast<double>(SDL_GetTicks() - frame_start); elapsed_time < kFrameTimeMs) {
      SDL_Delay(static_cast<uint32_t>(kFrameTimeMs - elapsed_time));
    }
  }
}

void NesApp::DrawMenuBar() {
  if (!ImGui::BeginMainMenuBar()) {
    return;
  }

  // The real height is only known after the first NewFrame (when the font size
  // is set), so the window starts at DEFAULT_MENU_BAR_HEIGHT and is resized
  // here only if the actual menu bar differs.
  if (const float height = std::ceil(ImGui::GetWindowHeight()); height != menu_bar_height_) {
    menu_bar_height_ = height;
    SDL_SetWindowSize(static_cast<SDL_Window*>(window_), WindowWidth(), WindowHeight());
  }

  const bool has_rom = emulator_ != nullptr;

  if (ImGui::BeginMenu("File")) {
    if (ImGui::MenuItem("Open ROM", "Ctrl+O")) {
      ShowOpenRomDialog();
    }
    if (ImGui::MenuItem("Reset", "Ctrl+R", false, has_rom)) {
      Reset();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Exit", "Esc")) {
      running_ = false;
    }
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("State")) {
    if (ImGui::MenuItem("Save State", "F5", false, has_rom)) {
      SaveState();
    }
    if (ImGui::MenuItem("Load State", "F9", false, has_rom)) {
      LoadState();
    }
    ImGui::EndMenu();
  }

  ImGui::EndMainMenuBar();
}

void NesApp::Render(const std::vector<uint8_t>* frame_buffer) {
  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer_);

  if (frame_buffer != nullptr) {
    SDL_UpdateTexture(texture_, nullptr, frame_buffer->data(), nes::Ppu::WIDTH * nes::Ppu::BYTES_PER_PIXEL);
    // Fit the picture below the menu bar, keeping the NES aspect ratio and
    // centering it with black bars if the window's shape doesn't match.
    int output_width = 0;
    int output_height = 0;
    SDL_GetCurrentRenderOutputSize(renderer_, &output_width, &output_height);
    const float area_width = static_cast<float>(output_width);
    const float area_height = std::max(0.0f, static_cast<float>(output_height) - menu_bar_height_);
    const float fit = std::min(area_width / static_cast<float>(nes::Ppu::WIDTH),
                               area_height / static_cast<float>(nes::Ppu::HEIGHT));
    const float width = static_cast<float>(nes::Ppu::WIDTH) * fit;
    const float height = static_cast<float>(nes::Ppu::HEIGHT) * fit;
    const SDL_FRect dest{(area_width - width) / 2.0f, menu_bar_height_ + (area_height - height) / 2.0f, width, height};
    SDL_RenderTexture(renderer_, texture_, nullptr, &dest);
  }

  ImGui::Render();
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer_);
  SDL_RenderPresent(renderer_);
}

int NesApp::WindowWidth() const { return nes::Ppu::WIDTH * scale_; }

int NesApp::WindowHeight() const { return nes::Ppu::HEIGHT * scale_ + static_cast<int>(menu_bar_height_); }

void NesApp::LoadRom(const std::string& rom_path) {
  emulator_ = std::make_unique<nes::Emulator>(rom_path);
  rom_path_ = rom_path;
  save_state_path_ = rom_path + ".state";
  SDL_ClearAudioStream(audio_stream_);

  const std::string title = std::string(kWindowTitle) + " - " + FileName(rom_path);
  SDL_SetWindowTitle(static_cast<SDL_Window*>(window_), title.c_str());
}

void NesApp::OpenRom(const std::string& rom_path) {
  try {
    LoadRom(rom_path);
  } catch (const std::exception& e) {
    ShowError("Failed to load ROM: " + std::string(e.what()));
  }
}

void NesApp::ShowOpenRomDialog() {
  SDL_ShowOpenFileDialog(OnOpenRomDialogResult, this, static_cast<SDL_Window*>(window_), kRomFilters,
                         static_cast<int>(std::size(kRomFilters)), nullptr, false);
}

void SDLCALL NesApp::OnOpenRomDialogResult(void* userdata, const char* const* filelist, int /*filter*/) {
  if (filelist == nullptr) {
    std::cerr << "Open Rom Dialog failed: " << SDL_GetError() << '\n';
    return;
  }
  if (*filelist == nullptr) {
    return; // user canceled
  }

  auto* app = static_cast<NesApp*>(userdata);
  const std::lock_guard lock(app->pending_rom_mutex_);
  app->pending_rom_path_ = *filelist;
}

void NesApp::ApplyPendingRom() {
  std::optional<std::string> rom_path;
  {
    const std::lock_guard lock(pending_rom_mutex_);
    rom_path = std::exchange(pending_rom_path_, std::nullopt);
  }
  // Load outside the lock so the dialog thread is never blocked on file I/O.
  if (rom_path.has_value()) {
    OpenRom(*rom_path);
  }
}

void NesApp::Reset() {
  // The core has no soft reset yet, so this is a power cycle.
  if (emulator_ != nullptr) {
    OpenRom(rom_path_);
  }
}

void NesApp::SaveState() {
  if (emulator_ == nullptr) {
    return;
  }
  try {
    emulator_->SaveStateToFile(save_state_path_);
  } catch (const std::exception& e) {
    ShowError("Failed to save state: " + std::string(e.what()));
  }
}

void NesApp::LoadState() {
  if (emulator_ == nullptr) {
    return;
  }
  try {
    emulator_->LoadStateFromFile(save_state_path_);
  } catch (const std::exception& e) {
    ShowError("Failed to load state: " + std::string(e.what()));
  }
}

void NesApp::ShowError(const std::string& message) const {
  std::cerr << message << '\n';
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, kWindowTitle, message.c_str(), static_cast<SDL_Window*>(window_));
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
    ImGui_ImplSDL3_ProcessEvent(&event);
    if (event.type == SDL_EVENT_QUIT) {
      running_ = false;
      continue;
    }

    if (event.type != SDL_EVENT_KEY_DOWN && event.type != SDL_EVENT_KEY_UP) {
      continue;
    }

    const bool is_down = event.type == SDL_EVENT_KEY_DOWN;
    const bool ui_has_keyboard = ImGui::GetIO().WantCaptureKeyboard;

    if (is_down && !event.key.repeat && !ui_has_keyboard) {
      const bool ctrl = (event.key.mod & SDL_KMOD_CTRL) != 0;
      switch (event.key.scancode) {
      case SDL_SCANCODE_ESCAPE:
        running_ = false;
        break;
      case SDL_SCANCODE_F5:
        SaveState();
        break;
      case SDL_SCANCODE_F9:
        LoadState();
        break;
      case SDL_SCANCODE_O:
        if (ctrl) {
          ShowOpenRomDialog();
        }
        break;
      case SDL_SCANCODE_R:
        if (ctrl) {
          Reset();
        }
        break;
      default:
        break;
      }
    }

    if (emulator_ == nullptr) {
      continue;
    }
    const auto& key_map = KeyMap();
    if (auto it = key_map.find(event.key.scancode); it != key_map.end()) {
      // Releases always go through so a button can't stick if the UI grabbed the keyboard mid-press.
      if (!is_down) {
        emulator_->GetBus().GetController1().Release(it->second);
      } else if (!ui_has_keyboard) {
        emulator_->GetBus().GetController1().Press(it->second);
      }
    }
  }
}

} // namespace nes_app