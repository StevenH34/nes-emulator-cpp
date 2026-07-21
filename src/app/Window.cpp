#include "Window.h"

#include <stdexcept>

namespace nes {

Window::Window(const std::string& title, const int width, const int height) {
    // SDL_CreateWindow(title, width, height, SDL_WindowFlag)
    ptr_ = SDL_CreateWindow(title.c_str(), width, height, 0);
    if (!ptr_) {
        throw std::runtime_error("Window creation failed: " + std::string(SDL_GetError()));
    }
    // Need to center window
    SDL_SetWindowPosition(ptr_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

Window::~Window() {
    if (ptr_) {
        SDL_DestroyWindow(ptr_);
    }
}

Window::Window(Window&& other) noexcept : ptr_(other.ptr_) {
    other.ptr_ = nullptr;
}

Window &Window::operator=(Window &&other) noexcept {
    if (this != &other) {
        if (ptr_) {
            SDL_DestroyWindow(ptr_);
        }
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
    }
    return *this;
}

}

