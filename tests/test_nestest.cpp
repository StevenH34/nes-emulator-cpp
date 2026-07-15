#include "doctest.h"
#include "TestRom.h"

#include "../src/Emulator.h"

#include <fstream>
#include <string>
#include <vector>

namespace {

template <typename T>
T parse_hex(const std::string& line, const std::size_t pos, const std::size_t len) {
    return static_cast<T>(std::stoul(line.substr(pos, len), nullptr, 16));
}

std::vector<std::string> read_log_lines(const std::string& path) {
    std::ifstream log_file(path);
    std::vector<std::string> log_lines;
    std::string line;
    while (std::getline(log_file, line)) {
        log_lines.push_back(line);
    }
    return log_lines;
}

} // namespace

TEST_CASE("nestest: CPU matches log for all official opcodes") {
    const auto nestest_file_path = std::string(TEST_ROMS_DIR) + "nestest.nes";
    const auto nestest_log_path = std::string(TEST_ROMS_DIR) + "nestest.log";
    nes::Emulator emulator(nestest_file_path);
    emulator.GetCpu().SetProgramCounter(0xC000);
    auto log_lines = read_log_lines(nestest_log_path);

    for (const auto& line : log_lines) {
        INFO("Line: " << line);
        auto cpu = emulator.GetCpu();
        REQUIRE(cpu.GetProgramCounter() == parse_hex<std::uint16_t>(line, 0, 4));
        REQUIRE(cpu.GetAccumulator() == parse_hex<std::uint8_t>(line, 50, 2));
        REQUIRE(cpu.GetXRegister() == parse_hex<std::uint8_t>(line, 55, 2));
        REQUIRE(cpu.GetYRegister() == parse_hex<std::uint8_t>(line, 60, 2));
        REQUIRE(cpu.GetStatusRegister() == parse_hex<std::uint8_t>(line, 65, 2));
        REQUIRE(cpu.GetStackPointer() == parse_hex<std::uint8_t>(line, 71, 2));

        try {
            emulator.Step();
        } catch (const nes::UnknownOpcode&) {
            MESSAGE("Stopped on UnknownOpcode at line: " << line);
            break;
        }
    }
}