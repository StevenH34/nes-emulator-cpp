#include "doctest.h"

#include "../src/core/save_state/StateWriter.h"

#include <array>
#include <bit>
#include <cstdint>
#include <vector>

TEST_CASE("StateWriter WriteU8 appends a single byte to the buffer") {
  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);

  writer.WriteU8(0x42);

  REQUIRE(buffer.size() == 1);
  CHECK(buffer[0] == 0x42);
}

TEST_CASE("StateWriter WriteU8 appends multiple bytes in call order") {
  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);

  writer.WriteU8(0x01);
  writer.WriteU8(0x02);
  writer.WriteU8(0x03);

  REQUIRE(buffer.size() == 3);
  CHECK(buffer[0] == 0x01);
  CHECK(buffer[1] == 0x02);
  CHECK(buffer[2] == 0x03);
}

TEST_CASE("StateWriter WriteU16 appends two bytes in little-endian order") {
  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);

  writer.WriteU16(0xABCD);

  REQUIRE(buffer.size() == 2);
  CHECK(buffer[0] == 0xCD); // low byte first
  CHECK(buffer[1] == 0xAB); // high byte second
}

TEST_CASE("StateWriter WriteU16 handles a value with a zero low byte") {
  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);

  writer.WriteU16(0x1200);

  REQUIRE(buffer.size() == 2);
  CHECK(buffer[0] == 0x00);
  CHECK(buffer[1] == 0x12);
}

TEST_CASE("StateWriter WriteU16 round-trips the maximum uint16 value") {
  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);

  writer.WriteU16(0xFFFF);

  REQUIRE(buffer.size() == 2);
  CHECK(buffer[0] == 0xFF);
  CHECK(buffer[1] == 0xFF);
}

TEST_CASE("StateWriter WriteU32 appends four bytes in little-endian order") {
  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);

  writer.WriteU32(0x12345678);

  REQUIRE(buffer.size() == 4);
  CHECK(buffer[0] == 0x78);
  CHECK(buffer[1] == 0x56);
  CHECK(buffer[2] == 0x34);
  CHECK(buffer[3] == 0x12);
}

TEST_CASE("StateWriter WriteU32 round-trips the maximum uint32 value") {
  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);

  writer.WriteU32(0xFFFFFFFF);

  REQUIRE(buffer.size() == 4);
  CHECK(buffer[0] == 0xFF);
  CHECK(buffer[1] == 0xFF);
  CHECK(buffer[2] == 0xFF);
  CHECK(buffer[3] == 0xFF);
}

TEST_CASE("StateWriter WriteU32 handles zero") {
  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);

  writer.WriteU32(0x00000000);

  REQUIRE(buffer.size() == 4);
  CHECK(buffer[0] == 0x00);
  CHECK(buffer[1] == 0x00);
  CHECK(buffer[2] == 0x00);
  CHECK(buffer[3] == 0x00);
}

TEST_CASE("StateWriter WriteBool writes a single byte, 1 for true") {
  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);

  writer.WriteBool(true);

  REQUIRE(buffer.size() == 1);
  CHECK(buffer[0] == 1);
}

TEST_CASE("StateWriter WriteBool writes a single byte, 0 for false") {
  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);

  writer.WriteBool(false);

  REQUIRE(buffer.size() == 1);
  CHECK(buffer[0] == 0);
}

TEST_CASE("StateWriter WriteFloat appends four bytes matching the IEEE-754 bit "
          "pattern, little-endian") {
  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);

  writer.WriteFloat(1.5f);

  const auto bits = std::bit_cast<uint32_t>(1.5f);
  REQUIRE(buffer.size() == 4);
  CHECK(buffer[0] == static_cast<uint8_t>(bits & 0xFF));
  CHECK(buffer[1] == static_cast<uint8_t>((bits >> 8) & 0xFF));
  CHECK(buffer[2] == static_cast<uint8_t>((bits >> 16) & 0xFF));
  CHECK(buffer[3] == static_cast<uint8_t>((bits >> 24) & 0xFF));
}

TEST_CASE("StateWriter WriteFloat handles negative values") {
  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);

  writer.WriteFloat(-735.0f);

  const auto bits = std::bit_cast<uint32_t>(-735.0f);
  REQUIRE(buffer.size() == 4);
  CHECK(buffer[0] == static_cast<uint8_t>(bits & 0xFF));
  CHECK(buffer[1] == static_cast<uint8_t>((bits >> 8) & 0xFF));
  CHECK(buffer[2] == static_cast<uint8_t>((bits >> 16) & 0xFF));
  CHECK(buffer[3] == static_cast<uint8_t>((bits >> 24) & 0xFF));
}

TEST_CASE("StateWriter WriteFloat handles zero") {
  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);

  writer.WriteFloat(0.0f);

  REQUIRE(buffer.size() == 4);
  CHECK(buffer[0] == 0x00);
  CHECK(buffer[1] == 0x00);
  CHECK(buffer[2] == 0x00);
  CHECK(buffer[3] == 0x00);
}

TEST_CASE("StateWriter WriteBytes appends a span of bytes unchanged and in order") {
  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);
  const std::array<uint8_t, 4> data{0x11, 0x22, 0x33, 0x44};

  writer.WriteBytes(data);

  REQUIRE(buffer.size() == 4);
  CHECK(buffer[0] == 0x11);
  CHECK(buffer[1] == 0x22);
  CHECK(buffer[2] == 0x33);
  CHECK(buffer[3] == 0x44);
}

TEST_CASE("StateWriter WriteBytes with an empty span appends nothing") {
  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);
  const std::array<uint8_t, 0> data{};

  writer.WriteBytes(data);

  CHECK(buffer.empty());
}

TEST_CASE("StateWriter preserves earlier writes when later calls append more data") {
  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);

  writer.WriteU8(0xAA);
  REQUIRE(buffer.size() == 1);
  REQUIRE(buffer[0] == 0xAA);

  writer.WriteU16(0xBBCC);

  REQUIRE(buffer.size() == 3);
  CHECK(buffer[0] == 0xAA); // untouched by the later WriteU16 call
  CHECK(buffer[1] == 0xCC);
  CHECK(buffer[2] == 0xBB);
}

TEST_CASE("StateWriter appends to a buffer without clearing its existing contents") {
  std::vector<uint8_t> buffer{0x01, 0x02, 0x03};
  nes::StateWriter writer(buffer);

  writer.WriteU8(0x04);

  // The writer must never clear/own the buffer -- reuse across calls (e.g. a
  // netcode hot path calling buffer.clear() itself between frames) depends on
  // the writer only ever appending.
  REQUIRE(buffer.size() == 4);
  CHECK(buffer[0] == 0x01);
  CHECK(buffer[1] == 0x02);
  CHECK(buffer[2] == 0x03);
  CHECK(buffer[3] == 0x04);
}

TEST_CASE("StateWriter writes are visible through the caller's own buffer reference") {
  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);

  writer.WriteU32(0xDEADBEEF);

  // Confirms the writer operates on the caller-owned vector itself, not a
  // private copy -- essential for the reusable-buffer / zero-allocation
  // contract the netcode hot path relies on.
  REQUIRE(buffer.size() == 4);
  CHECK(buffer[0] == 0xEF);
  CHECK(buffer[1] == 0xBE);
  CHECK(buffer[2] == 0xAD);
  CHECK(buffer[3] == 0xDE);
}

TEST_CASE("StateWriter mixed-type writes produce the exact concatenated wire format") {
  std::vector<uint8_t> buffer;
  nes::StateWriter writer(buffer);

  writer.WriteU8(0x7F);
  writer.WriteBool(true);
  writer.WriteU16(0x0102);
  writer.WriteBool(false);
  writer.WriteU32(0x0A0B0C0D);

  const std::vector<uint8_t> expected{
      0x7F, // WriteU8
      0x01, // WriteBool(true)
      0x02, 0x01, // WriteU16, little-endian
      0x00, // WriteBool(false)
      0x0D, 0x0C, 0x0B, 0x0A, // WriteU32, little-endian
  };
  CHECK(buffer == expected);
}
