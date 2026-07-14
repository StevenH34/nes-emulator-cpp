#ifndef NES_EMULATOR_CPP_TEST_ROM_H
#define NES_EMULATOR_CPP_TEST_ROM_H

#include <cstdint>
#include <string>
#include <vector>

#include "../src/Cartridge.h"

namespace nes_test {

// Writes a byte buffer to a uniquely-named temp file and deletes it on destruction,
// so tests get an isolated .nes file on disk for Cartridge's file-based constructor.
class TempRomFile {
public:
    explicit TempRomFile(const std::vector<std::uint8_t>& data);
    ~TempRomFile();

    TempRomFile(const TempRomFile&) = delete;
    TempRomFile& operator=(const TempRomFile&) = delete;

    [[nodiscard]] const std::string& path() const { return path_; }

private:
    std::string path_;
};

// Builds a minimal mapper-0 (NROM) ROM: one 16 KB PRG bank, one 8 KB CHR bank, no
// trainer, all zeroed except for the reset vector (mirrored at CPU 0xFFFC/0xFFFD).
std::vector<std::uint8_t> MakeMinimalRom(std::uint16_t reset_vector = 0x0000);

// A process-wide Cartridge backed by a minimal ROM with reset vector 0x0000, for
// tests that need a Bus/Cpu but don't care about cartridge contents.
nes::Cartridge& GetTestCartridge();

} // nes_test

#endif //NES_EMULATOR_CPP_TEST_ROM_H
