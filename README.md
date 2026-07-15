### NES Emulator (in C++)

- ✅ Added All 151 official opcodes with instructions.
- ✅ Added Cartridge and Mapper000 support.
- ✅ Added nestest to verify all 151 opcodes are working correctly.
- 💀 Next step: add the PPU.

### Testing

Using `doctest.h`.

`cmake --build cmake-build-debug --target run_tests`

### Resources

- [Building Your First Emulator by Matias Salles](https://leanpub.com/nes-emulator-en)
- [OneLoneCoder/olcNES - GitHub](https://github.com/OneLoneCoder/olcNES)
- [6502 Instruction Reference](https://www.nesdev.org/obelisk-6502-guide/reference.html?__cf_chl_f_tk=z6uyc9XSsj2aWhSthPoDHP6SSNSXVHT0jTnPmVBZycc-1783039093-1.0.1.1-Rxg3fg7plNMeW95x2ZbAMl43kuHGWjQSOfGkQ1jImAA)
- [NMOS 6502 Opcodes by John Pickens](https://6502.org/tutorials/6502opcodes.html)