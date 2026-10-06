#include "FileIo.h"

#include <filesystem>
#include <format>
#include <fstream>
#include <random>
#include <stdexcept>
#include <system_error>

namespace nes {

namespace {

// Random per-call suffix, so concurrent saves to the same path never share a temp file.
uint64_t RandomSuffix() {
  thread_local std::mt19937_64 generator{std::random_device{}()};
  return generator();
}

void RemoveQuietly(const std::string& path) {
  std::error_code ignored;
  std::filesystem::remove(path, ignored);
}

} // namespace

void WriteFileAtomically(const std::string& path, const std::span<const uint8_t> bytes) {
  const std::string temp_path = std::format("{}.{:016x}.tmp", path, RandomSuffix());
  {
    std::ofstream file_stream(temp_path, std::ios::binary);
    if (!file_stream) {
      throw std::runtime_error(std::format("Failed to open file for writing: {}", path));
    }
    file_stream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    // Close explicitly so a failed final flush is reported instead of ignored by the destructor
    file_stream.close();
    if (!file_stream) {
      RemoveQuietly(temp_path);
      throw std::runtime_error(std::format("Failed to write to file: {}", path));
    }
  }

  std::error_code ec;
  std::filesystem::rename(temp_path, path, ec);
  if (ec) {
    RemoveQuietly(temp_path);
    throw std::runtime_error(std::format("Failed to replace {}: {}", path, ec.message()));
  }
}

} // namespace nes
