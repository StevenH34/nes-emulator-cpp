#include "WasmApp.h"

#include <cstdint>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <exception>
#include <string>
#include <vector>

namespace {

// Takes a JS Uint8Array. Returns an empty string on success, or an error message.
std::string LoadRom(const emscripten::val& rom_bytes) {
  nes_wasm::WasmApp* app = nes_wasm::GetActiveApp();
  if (app == nullptr) {
    return "Emulator is not initialized";
  }

  const auto bytes = emscripten::convertJSArrayToNumberVector<uint8_t>(rom_bytes);
  try {
    app->LoadRom(bytes);
  } catch (const std::exception& e) {
    return e.what();
  }
  return "";
}

} // namespace

EMSCRIPTEN_BINDINGS(nes_emulator) { emscripten::function("loadRom", &LoadRom); }
