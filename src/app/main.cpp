#include "NesApp.h"

#include <cstdio>

int main(const int argc, char* argv[]) {
    if (argc < 2) {
        std::fprintf(stderr, "Usage: %s <rom_path>\n", argv[0]);
        return 1;
    }

    try {
        nes_app::NesApp app(argv[1]);
        app.Run();
    }
    catch (const std::exception& e) {
        std::fprintf(stderr, "Error: %s\n", e.what());
        return 1;
    }

    return 0;
}
