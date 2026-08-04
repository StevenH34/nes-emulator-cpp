#ifndef NES_EMULATOR_CPP_APU_H
#define NES_EMULATOR_CPP_APU_H
#include <cstdint>


class Apu {
public:

private:
    static constexpr std::uint32_t SAMPLE_RATE = 44100;
    static constexpr std::uint32_t CPU_CLOCK = 1789773;
    // Used for downsampling
    static constexpr float CYCLES_PER_SAMPLE = static_cast<float>(CPU_CLOCK) / SAMPLE_RATE;
    // Pulse 1 registers
    static constexpr std::uint16_t PULSE1_MAIN_REGISTER       = 0x4000;
    static constexpr std::uint16_t PULSE1_SWEEP_REGISTER      = 0x4001;
    static constexpr std::uint16_t PULSE1_TIMER_LOW_REGISTER  = 0x4002;
    static constexpr std::uint16_t PULSE1_TIMER_HIGH_REGISTER = 0x4003;
    // Pulse 2 registers
    static constexpr std::uint16_t PULSE2_MAIN_REGISTER       = 0x4004;
    static constexpr std::uint16_t PULSE2_SWEEP_REGISTER      = 0x4005;
    static constexpr std::uint16_t PULSE2_TIMER_LOW_REGISTER  = 0x4006;
    static constexpr std::uint16_t PULSE2_TIMER_HIGH_REGISTER = 0x4007;
    // Triangle registers
    static constexpr std::uint16_t TRIANGLE_LINEAR_REGISTER     = 0x4008;
    static constexpr std::uint16_t TRIANGLE_TIMER_LOW_REGISTER  = 0x400A;
    static constexpr std::uint16_t TRIANGLE_TIMER_HIGH_REGISTER = 0x400B;
    // Noise registers
    static constexpr std::uint16_t NOISE_MAIN_REGISTER        = 0x400C;
    static constexpr std::uint16_t NOISE_MODE_PERIOD_REGISTER = 0x400E;
    static constexpr std::uint16_t NOISE_LENGTH_REGISTER      = 0x400F;
    // Frame counter step sequence
    struct FrameCounterNtsc {
        // Mode 0: 4-Step Sequence
        static constexpr std::uint32_t MODE0_STEP1 = 7457;  // 3728.5 * 2
        static constexpr std::uint32_t MODE0_STEP2 = 14914; // 7456.5 * 2 (Rounded)
        static constexpr std::uint32_t MODE0_STEP3 = 22371; // 11185.5 * 2
        static constexpr std::uint32_t MODE0_STEP4 = 29829; // 14914.5 * 2
        static constexpr std::uint32_t MODE0_MAX   = 29830; // Total sequence length

        // Mode 1: 5-Step Sequence
        static constexpr std::uint32_t MODE1_STEP1 = 7457;  // 3728.5 * 2
        static constexpr std::uint32_t MODE1_STEP2 = 14914; // 7456.5 * 2
        static constexpr std::uint32_t MODE1_STEP3 = 22371; // 11185.5 * 2
        static constexpr std::uint32_t MODE1_STEP4 = 29829; // 14914.5 * 2
        static constexpr std::uint32_t MODE1_STEP5 = 37281; // 18640.5 * 2
        static constexpr std::uint32_t MODE1_MAX   = 37282; // Total sequence length
    };
    // Status register bit masks
    static constexpr std::uint8_t STATUS_PULSE1_MASK = 0x01;
    static constexpr std::uint8_t STATUS_PULSE2_MASK = 0x02;
    static constexpr std::uint8_t STATUS_TRIANGLE_MASK = 0x04;
    static constexpr std::uint8_t STATUS_NOISE_MASK    = 0x08;
    // Mixer Coefficients
    static constexpr float PULSE_MIXER_COEFFICIENT = 0.00752f;
    static constexpr float TND_T_MIXER_COEFFICIENT = 0.00851f;
    static constexpr float TND_N_MIXER_COEFFICIENT = 0.00494f;
    // Length counter look-up table
    static constexpr std::uint8_t LENGTH_COUNTER_TABLE[32] = {
        10, 254,  20,   2,  40,   4,  80,   6,  160,   8,  60,  10,  14,  12,  26,  14,
        12,  16,  24,  18,  48,  20,  96,  22,  192,  24,  72,  26,  16,  28,  32,  30
    };
};


#endif //NES_EMULATOR_CPP_APU_H
