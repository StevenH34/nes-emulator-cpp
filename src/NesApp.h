#ifndef NES_EMULATOR_CPP_NES_APP_H
#define NES_EMULATOR_CPP_NES_APP_H

#include <string>

#include "Emulator.h"
#include "Window.h"
#include <stdexcept>

namespace nes_app {

class NesApp {
public:
    explicit NesApp(const std::string& rom_path);
    ~NesApp();
    // Non-copyable
    NesApp(const NesApp&) = delete;
    NesApp& operator=(const NesApp&) = delete;

    void Run();

    static constexpr int SCALE = 3;
    static constexpr int WINDOW_WIDTH = nes::Ppu::WIDTH * SCALE;    // 768
    static constexpr int WINDOW_HEIGHT = nes::Ppu::HEIGHT * SCALE;  // 720

private:
    // Owns the SDL library lifetime; must outlive window_ and be
    // constructed before it, since window creation requires SDL_Init.
    struct SdlLifetime {
        SdlLifetime();
        ~SdlLifetime();
    };

    void HandleEvents();
    void Cleanup() const;

    nes::Emulator emulator_;
    bool running_{true};
    SdlLifetime sdl_;
    nes::Window window_;
    SDL_Renderer* renderer_{nullptr};
    SDL_Texture* texture_{nullptr};
};

} // namespace nes_app
#endif //NES_EMULATOR_CPP_NES_APP_H
