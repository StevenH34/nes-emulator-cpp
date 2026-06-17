#include "Emulator.h"

int main() {
    nes::Bus bus;
    nes::Cpu cpu(bus);
    nes::Emulator emulator(bus, cpu);
    emulator.Run();
    return 0;
}