#include "NesApp.h"

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <optional>
#include <string>

int main(int argc, char* argv[]) {
  if (argc > 2) {
    std::fprintf(stderr, "Usage: %s [rom_path]\n", argv[0]);
    return EXIT_FAILURE;
  }

  // With no ROM argument the app starts empty; use File > Open ROM.
  const std::optional<std::string> rom_path = argc == 2 ? std::optional<std::string>(argv[1]) : std::nullopt;

  try {
    nes_app::NesApp app(rom_path);
    app.Run();
  } catch (const std::exception& e) {
    std::fprintf(stderr, "Error: %s\n", e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
