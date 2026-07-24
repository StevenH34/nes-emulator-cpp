#include "NesApp.h"

#include <stdexcept>

namespace nes_app {

namespace {
// NTSC NES PPU runs at ~60.0988 Hz.
constexpr double kFrameTimeMs = 1000.0 / 60.0988;
}  // namespace

NesApp::SdlLifetime::SdlLifetime() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error("SDL_Init failed: " + std::string(SDL_GetError()));
    }
}

NesApp::SdlLifetime::~SdlLifetime() {
    SDL_Quit();
}

NesApp::NesApp(const std::string &rom_path)
    : emulator_(rom_path),
      window_("NES Emulator", WINDOW_WIDTH, WINDOW_HEIGHT) {
    try {
        renderer_ = SDL_CreateRenderer(static_cast<SDL_Window*>(window_), nullptr);
        if (!renderer_) {
            throw std::runtime_error("SDL_CreateRenderer failed: " + std::string(SDL_GetError()));
        }

        texture_ = SDL_CreateTexture(
            renderer_,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_STREAMING,
            nes::Ppu::WIDTH,
            nes::Ppu::HEIGHT
            );
        if (!texture_) {
            throw std::runtime_error("SDL_CreateTexture failed: " + std::string(SDL_GetError()));
        }

        // TODO: Add audio
    } catch (...) {
        Cleanup();
        throw;
    }
}

NesApp::~NesApp() {
    Cleanup();
}

void NesApp::Cleanup() const {
    // TODO: Destroy audio
    if (texture_) SDL_DestroyTexture(texture_);
    if (renderer_) SDL_DestroyRenderer(renderer_);
}

void NesApp::Run() {
    while (running_) {
        const std::uint64_t frame_start = SDL_GetTicks();

        HandleEvents();

        const auto& frame_buffer = emulator_.RunFrame();

        // TODO: Add audio

        SDL_UpdateTexture(texture_, nullptr, frame_buffer.data(), nes::Ppu::WIDTH * 4);
        SDL_RenderClear(renderer_);
        SDL_RenderTexture(renderer_, texture_, nullptr, nullptr);
        SDL_RenderPresent(renderer_);

        // ~60 FPS (NTSC)
        if (const double elapsed_time = static_cast<double>(SDL_GetTicks() - frame_start);
            elapsed_time < kFrameTimeMs) {
            SDL_Delay(static_cast<std::uint32_t>(kFrameTimeMs - elapsed_time));
        }
    }
}

void NesApp::HandleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT ||
            (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_ESCAPE)) {
            running_ = false;
        }
        if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
            if (auto it = KEY_MAP.find(event.key.scancode); it != KEY_MAP.end()) {
                if (event.type == SDL_EVENT_KEY_DOWN) {
                    emulator_.GetBus().GetController1().Press(it->second);
                } else {
                    emulator_.GetBus().GetController1().Release(it->second);
                }
            }
        }
    }
}

} // namespace nes
