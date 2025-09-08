#pragma once

#include <cstdint>
#include <random>

const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_WIDTH = 64;

class Chip8 {
public:
  void LoadROM(char const *filename);
  Chip8();
  void Cycle();

  uint8_t registers[16]{};
  uint8_t memory[4096]{};
  uint16_t index{};
  uint16_t pc{};
  uint16_t stack[16]{};
  uint8_t sp{};
  uint8_t delayTimer{};
  uint8_t soundTimer{};
  uint8_t keypad[16]{};
  uint32_t video[64 * 32]{};
  uint16_t opcode;

private:
  std::default_random_engine randGen;
  std::uniform_int_distribution<unsigned int> dist;

  void Table0();
  void Table8();
  void TableE();
  void TableF();

  void OP_NULL(); // Dummy function
  void OP_00E0(); // set memory to 0
  void OP_00EE(); // RET
  void OP_1nnn(); // JMP
  void OP_2nnn(); // CALL
  void OP_3xkk(); // SE Vx, byte
  void OP_4xkk(); // SNE Vx, byte
  void OP_5xy0(); // SE Vx, Vy
  void OP_6xkk(); // LD Vx, byte
  void OP_7xkk(); // ADD Vx, byte
  void OP_8xy0(); // LD Vx, Vy
  void OP_8xy1(); // OR Vx, Vy
  void OP_8xy2(); // AND Vx, Vy
  void OP_8xy3(); // XOR Vx, Vy
  void OP_8xy4(); // ADD Vx, Vy
  void OP_8xy5(); // SUB Vx, Vy
  void OP_8xy6(); // SHR Vx | Halve Vx and keep the least segnificant bit in
                  // VF(0 or 1)
  void OP_8xy7(); // SUBN Vx, Vy
  void OP_8xyE(); // SHL Vx {, Vy} | double Vx and keep LSB in VF
  void OP_9xy0(); // Skip next instruction if Vx == Vy
  void OP_Annn(); // Set indext to nnn
  void OP_Bnnn(); // Jmp to V0 + address
  void OP_Cxkk(); // RND Vx, byte
  void OP_Dxyn(); // Display n-byte sprite
  void OP_Ex9E(); // Skip if key is pressed
  void OP_ExA1(); // Skip if ket is not pressed
  void OP_Fx07(); // Set Vx to delayTimer value
  void OP_Fx0A(); // Wait for keypress and store it in Vx
  void OP_Fx15(); // Set delayTimer = Vx
  void OP_Fx18(); // Set soundTimer = Vx
  void OP_Fx1E(); // Set index = index + Vx
  void OP_Fx29(); // Set index to start of sprite in Vx
  void OP_Fx33(); // Represent Vx in index, index+1, index+2
  void OP_Fx55(); // Store every register from V0 till Vx into memory, starting
                  // at index
  void OP_Fx65(); // Read registers from V0-Vx from memory, starting at index

  typedef void (Chip8::*Chip8Func)(); // Defines Chip8Func as a pointer to a
                                      // function of the Chip8 class that
                                      // returns void and takes no parameters
  Chip8Func table[0xF + 1];
  Chip8Func table0[0xE + 1];
  Chip8Func table8[0xE + 1];
  Chip8Func tableE[0xE + 1];
  Chip8Func tableF[0x65 + 1];
};
