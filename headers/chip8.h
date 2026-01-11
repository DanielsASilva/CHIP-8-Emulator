#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>
#include <random>

class chip8 {
    private:
        
        uint16_t PC;        // Program Counter
        uint16_t I;         // Instruction register
        
        uint16_t SP;        // Stack pointer

        uint8_t DT;         // Delay timer
        uint8_t ST;         // Sound timer
        
        uint8_t V[16];      // General purpose registers


        uint8_t RAM[4096];  // Memory
        
        // Instruction decoding variables
        uint16_t opcode;
        uint8_t instruction;
        uint8_t X;
        uint8_t Y;
        uint8_t N;
        uint8_t NN;
        uint16_t NNN;

        // PRNG
        std::mt19937 mt{};
        std::uniform_int_distribution<uint8_t> rand8bit{};

    public:
        chip8();

        void readRAM();
        bool loadROM(char ROM[]);

        void fetch();
        void decode();
        void execute(bool modernShift, bool keys[]);

        void disassemble();
        void debug();

        void decreaseTimers();
        bool isBeeping();

        uint32_t VBUF[2048]; // Video buffer
};

#endif
