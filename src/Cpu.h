#ifndef CHIP_8_EMULATOR_CPU_H
#define CHIP_8_EMULATOR_CPU_H
#include <cstdint>

const unsigned int REGISTERS = 16;
const unsigned int STACK_LEVELS = 16;

struct Cpu {
    uint8_t registers[REGISTERS]{};
    uint16_t index{};
    uint16_t pc{};
    uint16_t stack[STACK_LEVELS]{};
    uint8_t sp{};
    uint8_t delay_timer{};
    uint8_t sound_timer{};
};

#endif //CHIP_8_EMULATOR_CPU_H
