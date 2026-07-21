#ifndef NES_EMULATOR_CPP_WINDOW_H
#define NES_EMULATOR_CPP_WINDOW_H

#include <string>
#include <SDL3/SDL.h>
#include <stdexcept>

namespace nes {

class Window {
public:
    Window(const std::string& title, int width, int height);
    ~Window();

    // Disable copying
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // Allowing moving
    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;

    explicit operator SDL_Window* () const { return ptr_; }
private:
    SDL_Window* ptr_ = nullptr;
};

}
#endif //NES_EMULATOR_CPP_WINDOW_H
