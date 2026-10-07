#ifndef NES_EMULATOR_CPP_TEST_ROM_H
#define NES_EMULATOR_CPP_TEST_ROM_H

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "../src/core/Cartridge.h"

namespace nes_test {

// Writes a byte buffer to a uniquely-named temp file and deletes it on
// destruction, so tests get an isolated .nes file on disk for Cartridge's
// file-based constructor.
class TempRomFile {
public:
  explicit TempRomFile(const std::vector<uint8_t>& data);
  ~TempRomFile();

  TempRomFile(const TempRomFile&) = delete;
  TempRomFile& operator=(const TempRomFile&) = delete;

  [[nodiscard]] const std::string& path() const { return path_; }

private:
  std::string path_;
};

// A new, uniquely-named directory under the system temp directory, removed
// with its contents on destruction.
class TempDir {
public:
  explicit TempDir(const std::string& prefix);
  ~TempDir();

  TempDir(const TempDir&) = delete;
  TempDir& operator=(const TempDir&) = delete;

  [[nodiscard]] const std::filesystem::path& path() const { return path_; }

private:
  std::filesystem::path path_;
};

// Names of the entries directly inside dir, sorted; used to check that no temp files were left behind.
std::vector<std::string> EntryNames(const std::filesystem::path& dir);

// Builds a minimal mapper-0 (NROM) ROM: one 16 KB PRG bank, one 8 KB CHR bank,
// no trainer, all zeroed except for the reset vector (mirrored at CPU
// 0xFFFC/0xFFFD).
std::vector<uint8_t> MakeMinimalRom(uint16_t reset_vector = 0x0000);

// A process-wide Cartridge backed by a minimal ROM with reset vector 0x0000,
// for tests that need a Bus/Cpu but don't care about cartridge contents.
nes::Cartridge& GetTestCartridge();

} // namespace nes_test

#endif // NES_EMULATOR_CPP_TEST_ROM_H
