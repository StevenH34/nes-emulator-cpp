#include "TestRom.h"

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <format>
#include <fstream>
#include <random>

namespace nes_test {

namespace {

// A temp-directory path that is unique across test processes as well as within one,
// so overlapping test runs (e.g. debug and ASan builds) never share files.
std::filesystem::path UniqueTempPath(const std::string& prefix, const std::string& extension = "") {
  static const uint64_t run_id = std::random_device{}() * 0x100000000ULL + std::random_device{}();
  static std::atomic<int> counter{0};
  return std::filesystem::temp_directory_path() / std::format("{}_{:016x}_{}{}", prefix, run_id, counter++, extension);
}

} // namespace

TempRomFile::TempRomFile(const std::vector<uint8_t>& data) {
  path_ = UniqueTempPath("nes_test_rom", ".nes").string();
  std::ofstream file(path_, std::ios::binary);
  file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
}

TempRomFile::~TempRomFile() {
  std::error_code ec;
  std::filesystem::remove(path_, ec);
}

TempDir::TempDir(const std::string& prefix) : path_(UniqueTempPath(prefix)) {
  std::filesystem::create_directory(path_);
}

TempDir::~TempDir() {
  std::error_code ec;
  std::filesystem::remove_all(path_, ec);
}

std::vector<std::string> EntryNames(const std::filesystem::path& dir) {
  std::vector<std::string> names;
  for (const auto& entry : std::filesystem::directory_iterator(dir)) {
    names.push_back(entry.path().filename().string());
  }
  std::ranges::sort(names);
  return names;
}

std::vector<uint8_t> MakeMinimalRom(const uint16_t reset_vector) {
  std::vector<uint8_t> data{0x4E, 0x45, 0x53, 0x1A, 0x01, 0x01, 0x00, 0x00, 0, 0, 0, 0, 0, 0, 0, 0};
  data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE + nes::Cartridge::CHR_BLOCK_SIZE, 0);

  // Reset vector lives in the last 2 bytes of the 16 KB PRG bank (CPU
  // 0xFFFC/0xFFFD, mirrored down to PRG offset 0x3FFC/0x3FFD by Mapper000's 16
  // KB mask).
  const auto prg_start = nes::Cartridge::HEADER_SIZE;
  data[prg_start + 0x3FFC] = static_cast<uint8_t>(reset_vector & 0xFF);
  data[prg_start + 0x3FFD] = static_cast<uint8_t>(reset_vector >> 8);

  return data;
}

nes::Cartridge& GetTestCartridge() {
  static const TempRomFile rom(MakeMinimalRom());
  static nes::Cartridge cartridge(rom.path());
  return cartridge;
}

} // namespace nes_test
