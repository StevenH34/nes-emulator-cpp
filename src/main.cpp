#include "Emulator.h"

#include <cstdio>

int main(const int argc, char* argv[]) {
    if (argc < 2) {
        std::fprintf(stderr, "Usage: %s <rom_path>\n", argv[0]);
        return 1;
    }

    nes::Emulator emulator(argv[1]);
    emulator.Run();
    return 0;
}