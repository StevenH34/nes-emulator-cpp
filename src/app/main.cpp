#include "NesApp.h"

#include <cstdio>
#include <cstdlib>
#include <exception>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::fprintf(stderr, "Usage: %s <rom_path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    try {
        nes_app::NesApp app(argv[1]);
        app.Run();
    }
    catch (const std::exception& e) {
        std::fprintf(stderr, "Error: %s\n", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
