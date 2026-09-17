#pragma once

#include <cstdint>
#include <cstring>
#include <span>
#include <vector>

namespace nes {
class StateWriter {
public:
  explicit StateWriter(std::vector<uint8_t>& buffer) : buffer_(buffer) {}
  void WriteU8(const uint8_t value) { buffer_.push_back(value); }
  // little-endian
  void WriteU16(const uint16_t value) {
    // Use `& 0xFF` to isolate the lower 8 bits and `>> 8` to shift the upper 8 bits down
    buffer_.push_back(static_cast<uint8_t>(value & 0xFF));
    buffer_.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
  }
  // little-endian
  void WriteU32(const uint32_t value) {
    buffer_.push_back(static_cast<uint8_t>(value & 0xFF));
    buffer_.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    buffer_.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    buffer_.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
  }
  // single 0/1 byte
  void WriteBool(const bool value) { buffer_.push_back(value ? 1 : 0); }
  // bit_cast<uint32_t> then WriteU32
  void WriteFloat(const float value) {
    uint32_t int_value;
    // Compile-time assertion to ensure that float and uint32_t are the same size
    static_assert(sizeof(float) == sizeof(uint32_t), "float and uint32_t must be the same size");
    // Copies the raw 4 bytes of the float into the uint32_t without changing the bit pattern.
    // `memcpy(dest, src, count)` copies `sizeof(float)` bytes from `&value` to `&int_value`.
    std::memcpy(&int_value, &value, sizeof(float));
    WriteU32(int_value);
  }
  void WriteBytes(std::span<const uint8_t> bytes) {
    buffer_.insert(buffer_.end(), bytes.begin(), bytes.end());
  }

private:
  std::vector<uint8_t>& buffer_;
};

} // namespace nes