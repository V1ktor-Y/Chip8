#include "Chip8.hpp"
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <random>

const unsigned int START_ADDRESS = 0x200;

const unsigned int FONTSET_START_ADDRESS = 0x50;
const unsigned int FONTSET_SIZE = 80;
uint8_t fontset[FONTSET_SIZE] = {
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
    : randGen(std::chrono::system_clock::now().time_since_epoch().count()) {
  dist = std::uniform_int_distribution<unsigned int>(0, 255U);
  pc = START_ADDRESS;

  // Load fonts into memory
  for (int i = 0; i < FONTSET_SIZE; ++i) {
    memory[FONTSET_START_ADDRESS + i] = fontset[i];
  }
}

void Chip8::LoadROM(char const *filename) {

  std::ifstream file(filename, std::ios::binary | std::ios::ate);

  if (file.is_open()) {
    std::streampos size = file.tellg();
    char *buffer = new char[size];

    file.seekg(0, std::ios::beg);
    file.read(buffer, size);
    file.close();

    for (int i = 0; i < size; ++i) {
      memory[START_ADDRESS + i] = buffer[i];
    }

    delete[] buffer;
  }
}

void Chip8::OP_00E0() { memset(memory, 0, sizeof(video)); }
void Chip8::OP_00EE() { pc = stack[--sp]; }
void Chip8::OP_1nnn() {
  uint16_t address = opcode & 0x0FFFu;
  pc = address;
}

void Chip8::OP_2nnn() {
  uint16_t address = opcode & 0x0FFFu;
  stack[sp] = pc;
  pc = address;
  sp++;
}

void Chip8::OP_3xkk() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t byte = opcode & 0x00FFu;
  if (registers[Vx] == byte) {
    pc += 2;
  }
}

void Chip8::OP_4xkk() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t byte = opcode & 0x00FFu;
  if (registers[Vx] != byte) {
    pc += 2;
  }
}

void Chip8::OP_5xy0() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  if (registers[Vx] == registers[Vy]) {
    pc += 2;
  }
}
void Chip8::OP_6xkk() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t byte = opcode & 0x00FFu;
  registers[Vx] = byte;
}
void Chip8::OP_7xkk() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t byte = opcode & 0x00FFu;
  registers[Vx] += byte;
}
void Chip8::OP_8xy0() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  registers[Vx] = registers[Vy];
}
void Chip8::OP_8xy1() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  registers[Vx] |= registers[Vy];
}
void Chip8::OP_8xy2() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  registers[Vx] &= registers[Vy];
}
void Chip8::OP_8xy3() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  registers[Vx] ^= registers[Vy];
}
void Chip8::OP_8xy4() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  uint16_t sum = registers[Vx] + registers[Vy];
  if (sum > 0xFF) {
    registers[0xF] = 1;
  } else {
    registers[0xF] = 0;
  }
  registers[Vx] = sum & 0xFFu;
}
void Chip8::OP_8xy5() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  if (registers[Vx] > registers[Vy]) {
    registers[0xF] = 1;
  } else {
    registers[0xF] = 0;
  }
  registers[Vx] -= registers[Vy];
}
void Chip8::OP_8xy6() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  registers[0xF] = (registers[Vx] & 0x1u);
  registers[Vx] >>= 1;
}
void Chip8::OP_8xy7() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  if (registers[Vy] > registers[Vx]) {
    registers[0xF] = 1;
  } else {
    registers[0xF] = 0;
  }
  registers[Vx] = registers[Vy] - registers[Vx];
}
void Chip8::OP_8xyE() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  // Save MSB in VF
  registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

  registers[Vx] <<= 1;
}
void Chip8::OP_9xy0() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  if (registers[Vx] != registers[Vy]) {
    pc += 2;
  }
}

void Chip8::OP_Annn() {
  uint16_t address = opcode & 0x0FFFu;
  index = address;
}

void Chip8::OP_Bnnn() {
  uint16_t address = opcode & 0x0FFFu;
  pc = address + registers[0];
}

void Chip8::OP_Cxkk() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t byte = opcode & 0x00FFu;

  registers[Vx] = static_cast<uint8_t>(dist(randGen)) & byte;
}
void Chip8::OP_Dxyn() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;
  uint8_t height = opcode & 0x000Fu;

  // Wrap if going beyond screen boundaries
  uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
  uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

  registers[0xF] = 0;

  for (unsigned int row = 0; row < height; ++row) {
    uint8_t spriteByte = memory[index + row];

    for (unsigned int col = 0; col < 8; ++col) {
      uint8_t spritePixel = spriteByte & (0x80u >> col);
      uint32_t *screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

      // Sprite pixel is on
      if (spritePixel) {
        // Screen pixel also on - collision
        if (*screenPixel == 0xFFFFFFFF) {
          registers[0xF] = 1;
        }

        // Effectively XOR with the sprite pixel
        *screenPixel ^= 0xFFFFFFFF;
      }
    }
  }
}
void Chip8::OP_Ex9E() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  uint8_t key = registers[Vx];

  if (keypad[key]) {
    pc += 2;
  }
}
void Chip8::OP_ExA1() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  uint8_t key = registers[Vx];

  if (!keypad[key]) {
    pc += 2;
  }
}

void Chip8::OP_Fx07() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  registers[Vx] = delayTimer;
}

void Chip8::OP_Fx0A() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  for (int i = 0; i < 0xF; i++) {
    if (keypad[i]) {
      registers[Vx] = i;
      return;
    }
  }
  pc -= 2;
}

void Chip8::OP_Fx15() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  delayTimer = registers[Vx];
}

void Chip8::OP_Fx18() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  soundTimer = registers[Vx];
}

void Chip8::OP_Fx1E() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  index += registers[Vx];
}

void Chip8::OP_Fx29() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t digit = registers[Vx];

  index = FONTSET_START_ADDRESS + 5 * digit;
}

void Chip8::OP_Fx33() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t number = registers[Vx];

  memory[index + 2] = number % 10;
  number /= 10;

  memory[index + 1] = number % 10;
  number /= 10;

  memory[index] = number % 10;
}

void Chip8::OP_Fx55() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  for (int i = 0; i <= Vx; i++) {
    memory[index + i] = registers[i];
  }
}
void Chip8::OP_Fx65() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  for (int i = 0; i <= Vx; ++i) {
    registers[i] = memory[index + i];
  }
}
