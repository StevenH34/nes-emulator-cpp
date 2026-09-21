#pragma once
#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>

namespace nes {

class StateReader {
public:
  explicit StateReader(std::span<const uint8_t> data) : data_(data) {}

  uint8_t ReadU8() {
    if (cursor_ >= data_.size())
      throw std::out_of_range("StateReader: ReadU8 out of range");
    return data_[cursor_++];
  }

  uint16_t ReadU16() {
    RequireBytes(2);
    const uint16_t value = static_cast<uint16_t>(data_[cursor_] | (data_[cursor_ + 1] << 8));
    cursor_ += 2;
    return value;
  }

  uint32_t ReadU32() {
    RequireBytes(4);
    const uint32_t value = static_cast<uint32_t>(data_[cursor_] | (data_[cursor_ + 1] << 8) |
                                                 (data_[cursor_ + 2] << 16) | (data_[cursor_ + 3] << 24));
    cursor_ += 4;
    return value;
  }

  bool ReadBool() { return ReadU8() != 0; }

  float ReadFloat() {
    const uint32_t int_value = ReadU32();
    float value;
    // Compile-time assertion to ensure that float and uint32_t are the same size
    static_assert(sizeof(float) == sizeof(uint32_t), "float and uint32_t must be the same size");
    // Copies the raw 4 bytes of the uint32_t into the float without changing the bit pattern.
    std::memcpy(&value, &int_value, sizeof(float));
    return value;
  }

  // `memcpy` into caller's span, no alloc
  void ReadBytes(std::span<uint8_t> out) {
    RequireBytes(out.size());
    if (!out.empty()) {
      std::memcpy(out.data(), data_.data() + cursor_, out.size());
    }
    cursor_ += out.size();
  }

  [[nodiscard]] size_t BytesRemaining() const { return data_.size() - cursor_; }

private:
  std::span<const uint8_t> data_;
  size_t cursor_{0};

  void RequireBytes(const size_t count) const {
    if (cursor_ + count > data_.size())
      throw std::out_of_range("StateReader: RequireBytes out of range");
  }
};

} // namespace nes