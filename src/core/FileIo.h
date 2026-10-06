#pragma once

#include <cstdint>
#include <span>
#include <string>

namespace nes {

/* Writes bytes to a uniquely named temp file next to path, then renames it over path.
 * If the write or rename fails, any existing file at path is left untouched and the
 * temp file is removed, so no partial file is left behind. Throws std::runtime_error
 * on failure.
 * Not covered: a power loss mid-save. The temp file isn't flushed to disk before the
 * rename, so the new file can end up empty or partial after a crash of the OS.
 */
void WriteFileAtomically(const std::string& path, std::span<const uint8_t> bytes);

} // namespace nes
