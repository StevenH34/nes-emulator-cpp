#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/string.h> // IWYU pragma: keep (std::string <-> str caster for nb::init<std::string>)

#include "Bus.h"
#include "Cartridge.h"
#include "Controller.h"
#include "Emulator.h"
#include "ai/Preprocess.h"
#include "apu/Apu.h"
#include "ppu/Ppu.h"

#include <cstddef> // IWYU pragma: keep (std::size_t)
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace nb = nanobind;

namespace {

using UInt8Array = nb::ndarray<nb::numpy, uint8_t>;

constexpr std::size_t SCREEN_WIDTH = nes::Ppu::WIDTH;
constexpr std::size_t SCREEN_HEIGHT = nes::Ppu::HEIGHT;
constexpr std::size_t BYTES_PER_PIXEL = nes::Ppu::BYTES_PER_PIXEL;
constexpr std::size_t RGB_CHANNELS = 3;
constexpr std::size_t RAM_SIZE = 2048;

UInt8Array ToNumpy(std::vector<uint8_t> data, const std::initializer_list<std::size_t> list) {
  auto owned = std::make_unique<std::vector<uint8_t>>(std::move(data));
  const nb::capsule owner(owned.get(), [](void* p) noexcept { delete static_cast<std::vector<uint8_t>*>(p); });
  auto* raw = owned.release(); // capsule now owns the vector
  return UInt8Array(raw->data(), list, owner);
}

} // namespace

// Macro to generate the Python module for the NES emulator core
NB_MODULE(nes_py, m) { // NOLINT(performance-unnecessary-value-param): signature comes from nanobind's macro
  m.doc() = "Python bindings for NES emulator core";
  m.attr("OBS_SIZE") = nes::ai::OBS_SIZE;
  m.attr("BUTTON_A") = nes::Controller::BUTTON_A;
  m.attr("BUTTON_B") = nes::Controller::BUTTON_B;
  m.attr("BUTTON_SELECT") = nes::Controller::BUTTON_SELECT;
  m.attr("BUTTON_START") = nes::Controller::BUTTON_START;
  m.attr("BUTTON_UP") = nes::Controller::BUTTON_UP;
  m.attr("BUTTON_DOWN") = nes::Controller::BUTTON_DOWN;
  m.attr("BUTTON_LEFT") = nes::Controller::BUTTON_LEFT;
  m.attr("BUTTON_RIGHT") = nes::Controller::BUTTON_RIGHT;

  nb::class_<nes::Emulator>(m, "NesCore", "NES emulator instance for scripting and reinforcement learning.")
    .def(nb::init<std::string>(), nb::arg("rom_path"),
         "Load an iNES ROM from rom_path. Raises RuntimeError if the file can't be read or isn't a valid ROM.")

    .def(
      "step",
      [](nes::Emulator& emulator, const uint8_t buttons, // NOLINT(bugprone-easily-swappable-parameters)
         const int frames) {
        if (frames < 1)
          throw std::invalid_argument("frames must be >= 1");
        emulator.GetBus().GetController1().SetButtons(buttons);
        for (int i = 0; i < frames; ++i) {
          emulator.RunFrame();
          emulator.GetApu().GetSampleBuffer().clear(); // Drains audio
        }
      },
      nb::arg("buttons"), nb::kw_only(), nb::arg("frames") = 1, nb::call_guard<nb::gil_scoped_release>(),
      "Hold the controller-1 buttons (a bitmask of BUTTON_* constants) for the given number of frames, "
      "discarding audio. Releases the GIL while running. Raises ValueError if frames < 1.")

    .def(
      "frame",
      [](nes::Emulator& emulator) {
        const auto& rgba = emulator.GetPpu().GetFrameBuffer();
        std::vector<uint8_t> rgb(SCREEN_WIDTH * SCREEN_HEIGHT * RGB_CHANNELS);
        for (std::size_t i = 0, j = 0; i < rgba.size(); i += BYTES_PER_PIXEL, j += RGB_CHANNELS) {
          rgb[j] = rgba[i];
          rgb[j + 1] = rgba[i + 1];
          rgb[j + 2] = rgba[i + 2];
        }
        return ToNumpy(std::move(rgb), {SCREEN_HEIGHT, SCREEN_WIDTH, RGB_CHANNELS});
      },
      "Return the current screen as a (240, 256, 3) uint8 RGB array (a copy).")

    .def(
      "obs84",
      [](nes::Emulator& emulator) {
        const auto obs = nes::ai::ToObservation(emulator.GetPpu().GetFrameBuffer());
        return ToNumpy({obs.begin(), obs.end()}, {nes::ai::OBS_SIZE, nes::ai::OBS_SIZE});
      },
      "Return the current screen as an (84, 84) uint8 grayscale observation (a copy).")

    .def(
      "ram",
      [](nes::Emulator& emulator) {
        std::vector<uint8_t> ram(RAM_SIZE);
        for (std::size_t i = 0; i < RAM_SIZE; ++i) {
          ram[i] = emulator.GetBus().ReadRam(static_cast<uint16_t>(i));
        }
        return ToNumpy(std::move(ram), {RAM_SIZE});
      },
      "Return the 2 KB of CPU work RAM ($0000-$07FF) as a uint8 array (a copy). Reading it has no side effects.")

    .def(
      "save_state",
      [](const nes::Emulator& emulator) {
        const auto data = emulator.SaveStateToBytes();
        return nb::bytes(data.data(), data.size());
      },
      "Serialize the full emulator state to bytes.")

    .def(
      "load_state",
      [](nes::Emulator& emulator, const nb::bytes& state) {
        emulator.LoadStateFromBytes({static_cast<const uint8_t*>(state.data()), state.size()});
      },
      nb::arg("state"),
      "Restore a state from save_state(). Raises RuntimeError if the data is corrupt or comes from a different ROM.")

    .def(
      "rom_checksum", [](nes::Emulator& emulator) { return emulator.GetCartridge().GetRomChecksum(); },
      "Return the checksum of the loaded ROM's PRG and CHR data. Save states are tied to this value.");
}