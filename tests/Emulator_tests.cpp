#include "doctest.h"

#include <atomic>
#include <filesystem>
#include <fstream>
#include <vector>

#include "../src/Emulator.h"

namespace {

// Writes a byte buffer to a uniquely-named temp file and deletes it on destruction,
// so each test gets an isolated .nes file on disk for Emulator's file-based constructor.
class TempRomFile {
public:
    explicit TempRomFile(const std::vector<std::uint8_t>& data) {
        static std::atomic<int> counter{0};
        path_ = (std::filesystem::temp_directory_path() /
                 ("emulator_test_" + std::to_string(counter++) + ".nes")).string();
        std::ofstream file(path_, std::ios::binary);
        file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    }

    ~TempRomFile() {
        std::error_code ec;
        std::filesystem::remove(path_, ec);
    }

    TempRomFile(const TempRomFile&) = delete;
    TempRomFile& operator=(const TempRomFile&) = delete;

    [[nodiscard]] const std::string& path() const { return path_; }

private:
    std::string path_;
};

// Minimal mapper-0 (NROM) ROM: one 16 KB PRG bank, one 8 KB CHR bank, no trainer.
// The reset vector (mirrored at CPU address 0xFFFC/0xFFFD) is set to reset_vector.
std::vector<std::uint8_t> MakeRom(const std::uint16_t reset_vector) {
    std::vector<std::uint8_t> data{0x4E, 0x45, 0x53, 0x1A, 0x01, 0x01, 0x00, 0x00,
                                   0, 0, 0, 0, 0, 0, 0, 0};
    data.resize(data.size() + nes::Cartridge::PRG_BLOCK_SIZE + nes::Cartridge::CHR_BLOCK_SIZE, 0);

    // Reset vector lives in the last 2 bytes of the 16 KB PRG bank (CPU 0xFFFC/0xFFFD,
    // mirrored down to PRG offset 0x3FFC/0x3FFD by Mapper000's 16 KB mask).
    const auto prg_start = nes::Cartridge::HEADER_SIZE;
    data[prg_start + 0x3FFC] = static_cast<std::uint8_t>(reset_vector & 0xFF);
    data[prg_start + 0x3FFD] = static_cast<std::uint8_t>(reset_vector >> 8);

    return data;
}

}

TEST_CASE("Emulator::LoadProgram copies bytes into RAM") {
    const TempRomFile rom(MakeRom(0x0200));
    nes::Emulator emulator(rom.path());
    const std::vector<std::uint8_t> program{0xA9, 0x42, 0xE8};
    constexpr auto start_address = 0x0200;
    emulator.LoadProgram(program, start_address);

    CHECK(emulator.GetBus().ReadRam(0x0200) == 0xA9);
    CHECK(emulator.GetBus().ReadRam(0x0201) == 0x42);
    CHECK(emulator.GetBus().ReadRam(0x0202) == 0xE8);
}

TEST_CASE("Emulator constructor resets the CPU using the cartridge's reset vector") {
    const TempRomFile rom(MakeRom(0x0200));
    nes::Emulator emulator(rom.path());

    CHECK(emulator.GetCpu().GetProgramCounter() == 0x0200);
}

TEST_CASE("Emulator::Step executes the loaded program starting at the reset vector") {
    const TempRomFile rom(MakeRom(0x0200));
    nes::Emulator emulator(rom.path());
    const std::vector<std::uint8_t> program{0xA9, 0x42, 0xE8}; // LDA #0x42, INX
    emulator.LoadProgram(program, 0x0200);

    emulator.Step(); // LDA #0x42
    CHECK(emulator.GetCpu().GetAccumulator() == 0x42);

    emulator.Step(); // INX
    CHECK(emulator.GetCpu().GetXRegister() == 0x01);
}