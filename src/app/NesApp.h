#ifndef NES_EMULATOR_CPP_NES_APP_H
#define NES_EMULATOR_CPP_NES_APP_H

#include <string>

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
    static constexpr int WINDOW_WIDTH = nes::Ppu::WIDTH * SCALE;    // 768
    static constexpr int WINDOW_HEIGHT = nes::Ppu::HEIGHT * SCALE;  // 720

    void Run();

    // Key bindings
    static inline const std::unordered_map<SDL_Scancode, std::uint8_t> KEY_MAP = {{
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
