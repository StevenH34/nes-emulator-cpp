#include "WasmApp.h"

#include <cstdio>
#include <cstdlib>
#include <emscripten/emscripten.h>
#include <exception>
#include <memory>

namespace {

std::unique_ptr<nes_wasm::WasmApp> g_app;

void MainLoop(void* arg) { static_cast<nes_wasm::WasmApp*>(arg)->Tick(); }

} // namespace

nes_wasm::WasmApp* nes_wasm::GetActiveApp() { return g_app.get(); }

int main() {
  try {
    g_app = std::make_unique<nes_wasm::WasmApp>();
  } catch (const std::exception& e) {
    std::fprintf(stderr, "Error: %s\n", e.what());
    return EXIT_FAILURE;
  }

  // fps = 0 drives the loop from requestAnimationFrame. main() returns immediately; the runtime
  // stays alive (EXIT_RUNTIME is off by default) and keeps calling MainLoop.
  emscripten_set_main_loop_arg(MainLoop, g_app.get(), 0, false);
  return EXIT_SUCCESS;
}
