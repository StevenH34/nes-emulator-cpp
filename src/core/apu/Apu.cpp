#include "Apu.h"

namespace nes {

Apu::Apu() : pulse1_{0}, pulse2_{1}, triangle_{}, noise_{} { sample_buffer_.reserve(800); }

void Apu::Step() {}

} // namespace nes
