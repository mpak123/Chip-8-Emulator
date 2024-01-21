#include "Chip8.h"
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <random>

const unsigned int FONTSET_SIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x50;
const unsigned int START_ADDRESS = 0x200;


uint8_t fontset[FONTSET_SIZE] =
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

Chip8::Chip8()
    : rand_gen(std::chrono::system_clock::now().time_since_epoch().count())
{

    cpu.pc = START_ADDRESS;

    for (unsigned int i = 0; i < FONTSET_SIZE; ++i) {
		memory[FONTSET_START_ADDRESS + i] = fontset[i];
	}

    rand_byte = std::uniform_int_distribution<uint8_t>(0, 255U);

    table[0x0] = &Chip8::Table0;
	table[0x1] = &Chip8::OP_1nnn;
	table[0x2] = &Chip8::OP_2nnn;
	table[0x3] = &Chip8::OP_3xkk;
	table[0x4] = &Chip8::OP_4xkk;
	table[0x5] = &Chip8::OP_5xy0;
	table[0x6] = &Chip8::OP_6xkk;
	table[0x7] = &Chip8::OP_7xkk;
	table[0x8] = &Chip8::Table8;
	table[0x9] = &Chip8::OP_9xy0;
	table[0xA] = &Chip8::OP_Annn;
	table[0xB] = &Chip8::OP_Bnnn;
	table[0xC] = &Chip8::OP_Cxkk;
	table[0xD] = &Chip8::OP_Dxyn;
	table[0xE] = &Chip8::TableE;
	table[0xF] = &Chip8::TableF;

    for (size_t i = 0; i <= 0xE; i++) {
		table0[i] = &Chip8::OP_NULL;
		table8[i] = &Chip8::OP_NULL;
		tableE[i] = &Chip8::OP_NULL;
	}

    table0[0x0] = &Chip8::OP_00E0;
	table0[0xE] = &Chip8::OP_00EE;

	table8[0x0] = &Chip8::OP_8xy0;
	table8[0x1] = &Chip8::OP_8xy1;
	table8[0x2] = &Chip8::OP_8xy2;
	table8[0x3] = &Chip8::OP_8xy3;
	table8[0x4] = &Chip8::OP_8xy4;
	table8[0x5] = &Chip8::OP_8xy5;
	table8[0x6] = &Chip8::OP_8xy6;
	table8[0x7] = &Chip8::OP_8xy7;
	table8[0xE] = &Chip8::OP_8xyE;

	tableE[0x1] = &Chip8::OP_ExA1;
	tableE[0xE] = &Chip8::OP_Ex9E;

	for (size_t i = 0; i <= 0x65; i++) {
		tableF[i] = &Chip8::OP_NULL;
	}

	tableF[0x07] = &Chip8::OP_Fx07;
	tableF[0x0A] = &Chip8::OP_Fx0A;
	tableF[0x15] = &Chip8::OP_Fx15;
	tableF[0x18] = &Chip8::OP_Fx18;
	tableF[0x1E] = &Chip8::OP_Fx1E;
	tableF[0x29] = &Chip8::OP_Fx29;
	tableF[0x33] = &Chip8::OP_Fx33;
	tableF[0x55] = &Chip8::OP_Fx55;
	tableF[0x65] = &Chip8::OP_Fx65;
}

void Chip8::load(char const* filename) {
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (file.is_open()) {
		std::streampos size = file.tellg();
		char* buffer = new char[size];
		file.seekg(0, std::ios::beg);
		file.read(buffer, size);
		file.close();

		for (long i = 0; i < size; ++i) {
			memory[START_ADDRESS + i] = buffer[i];
		}

		delete[] buffer;
	}
}

void Chip8::cycle() {
	opcode = (memory[cpu.pc] << 8u) | memory[cpu.pc + 1];

	cpu.pc += 2;

	((*this).*(table[(opcode & 0xF000u) >> 12u]))();

	if (cpu.delay_timer > 0) {
		--cpu.delay_timer;
	}

	if (cpu.sound_timer > 0) {
		--cpu.sound_timer;
	}
}

void Chip8::Table0() {
	((*this).*(table0[opcode & 0x000Fu]))();
}

void Chip8::Table8() {
	((*this).*(table8[opcode & 0x000Fu]))();
}

void Chip8::TableE() {
	((*this).*(tableE[opcode & 0x000Fu]))();
}

void Chip8::TableF() {
	((*this).*(tableF[opcode & 0x00FFu]))();
}

void Chip8::OP_NULL() {}

void Chip8::OP_00E0() {
	memset(video, 0, sizeof(video));
}

void Chip8::OP_00EE() {
	--cpu.sp;
	cpu.pc = cpu.stack[cpu.sp];
}

void Chip8::OP_1nnn() {
	uint16_t address = opcode & 0x0FFFu;

	cpu.pc = address;
}

void Chip8::OP_2nnn() {
	uint16_t address = opcode & 0x0FFFu;

	cpu.stack[cpu.sp] = cpu.pc;
	++cpu.sp;
	cpu.pc = address;
}

void Chip8::OP_3xkk() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (cpu.registers[Vx] == byte) {
		cpu.pc += 2;
	}
}

void Chip8::OP_4xkk() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (cpu.registers[Vx] != byte) {
		cpu.pc += 2;
	}
}

void Chip8::OP_5xy0() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (cpu.registers[Vx] == cpu.registers[Vy]) {
		cpu.pc += 2;
	}
}

void Chip8::OP_6xkk() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	cpu.registers[Vx] = byte;
}

void Chip8::OP_7xkk() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	cpu.registers[Vx] += byte;
}

void Chip8::OP_8xy0() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	cpu.registers[Vx] = cpu.registers[Vy];
}

void Chip8::OP_8xy1() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	cpu.registers[Vx] |= cpu.registers[Vy];
}

void Chip8::OP_8xy2() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	cpu.registers[Vx] &= cpu.registers[Vy];
}

void Chip8::OP_8xy3() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	cpu.registers[Vx] ^= cpu.registers[Vy];
}

void Chip8::OP_8xy4() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	uint16_t sum = cpu.registers[Vx] + cpu.registers[Vy];

	if (sum > 255U) {
		cpu.registers[0xF] = 1;
	}
	else {
		cpu.registers[0xF] = 0;
	}

	cpu.registers[Vx] = sum & 0xFFu;
}

void Chip8::OP_8xy5() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (cpu.registers[Vx] > cpu.registers[Vy]) {
		cpu.registers[0xF] = 1;
	}
	else {
		cpu.registers[0xF] = 0;
	}

	cpu.registers[Vx] -= cpu.registers[Vy];
}

void Chip8::OP_8xy6() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// Save LSB in VF
	cpu.registers[0xF] = (cpu.registers[Vx] & 0x1u);

	cpu.registers[Vx] >>= 1;
}

void Chip8::OP_8xy7() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (cpu.registers[Vy] > cpu.registers[Vx]) {
		cpu.registers[0xF] = 1;
	}
	else {
		cpu.registers[0xF] = 0;
	}

	cpu.registers[Vx] = cpu.registers[Vy] - cpu.registers[Vx];
}

void Chip8::OP_8xyE() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// Save MSB in VF
	cpu.registers[0xF] = (cpu.registers[Vx] & 0x80u) >> 7u;

	cpu.registers[Vx] <<= 1;
}

void Chip8::OP_9xy0() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (cpu.registers[Vx] != cpu.registers[Vy]) {
		cpu.pc += 2;
	}
}

void Chip8::OP_Annn() {
	uint16_t address = opcode & 0x0FFFu;

	cpu.index = address;
}

void Chip8::OP_Bnnn() {
	uint16_t address = opcode & 0x0FFFu;

	cpu.pc = cpu.registers[0] + address;
}

void Chip8::OP_Cxkk() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	cpu.registers[Vx] = rand_byte(rand_gen) & byte;
}

void Chip8::OP_Dxyn() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t height = opcode & 0x000Fu;

	uint8_t xPos = cpu.registers[Vx] % WIDTH;
	uint8_t yPos = cpu.registers[Vy] % LENGTH;

	cpu.registers[0xF] = 0;

	for (unsigned int row = 0; row < height; ++row) {
		uint8_t spriteByte = memory[index + row];

		for (unsigned int col = 0; col < 8; ++col) {
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t* screenPixel = &video[(yPos + row) * 64 + (xPos + col)];

			if (spritePixel) {
				if (*screenPixel == 0xFFFFFFFF) {
					cpu.registers[0xF] = 1;
				}

				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
}

void Chip8::OP_Ex9E() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	uint8_t key = cpu.registers[Vx];

	if (keypad[key]) {
		cpu.pc += 2;
	}
}

void Chip8::OP_ExA1() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	uint8_t key = cpu.registers[Vx];

	if (!keypad[key]) {
		cpu.pc += 2;
	}
}

void Chip8::OP_Fx07() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	cpu.registers[Vx] = cpu.delay_timer;
}

void Chip8::OP_Fx0A() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	if (keypad[0]) {
		cpu.registers[Vx] = 0;
	}
	else if (keypad[1]) {
		cpu.registers[Vx] = 1;
	}
	else if (keypad[2]) {
		cpu.registers[Vx] = 2;
	}
	else if (keypad[3]) {
		cpu.registers[Vx] = 3;
	}
	else if (keypad[4]) {
		cpu.registers[Vx] = 4;
	}
	else if (keypad[5]) {
		cpu.registers[Vx] = 5;
	}
	else if (keypad[6]) {
		cpu.registers[Vx] = 6;
	}
	else if (keypad[7]) {
		cpu.registers[Vx] = 7;
	}
	else if (keypad[8]) {
		cpu.registers[Vx] = 8;
	}
	else if (keypad[9]) {
		cpu.registers[Vx] = 9;
	}
	else if (keypad[10]) {
		cpu.registers[Vx] = 10;
	}
	else if (keypad[11]) {
		cpu.registers[Vx] = 11;
	}
	else if (keypad[12]) {
		cpu.registers[Vx] = 12;
	}
	else if (keypad[13]) {
		cpu.registers[Vx] = 13;
	}
	else if (keypad[14]) {
		cpu.registers[Vx] = 14;
	}
	else if (keypad[15]) {
		cpu.registers[Vx] = 15;
	}
	else {
		cpu.pc -= 2;
	}
}

void Chip8::OP_Fx15() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	cpu.delay_timer = cpu.registers[Vx];
}

void Chip8::OP_Fx18() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	cpu.sound_timer = cpu.registers[Vx];
}

void Chip8::OP_Fx1E() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	cpu.index += cpu.registers[Vx];
}

void Chip8::OP_Fx29() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t digit = cpu.registers[Vx];

	cpu.index = FONTSET_START_ADDRESS + (5 * digit);
}

void Chip8::OP_Fx33() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t value = cpu.registers[Vx];

	memory[cpu.index + 2] = value % 10;
	value /= 10;

	memory[cpu.index + 1] = value % 10;
	value /= 10;

	memory[cpu.index] = value % 10;
}

void Chip8::OP_Fx55() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i)
	{
		memory[cpu.index + i] = cpu.registers[i];
	}
}

void Chip8::OP_Fx65() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i) {
		cpu.registers[i] = memory[cpu.index + i];
	}
}

