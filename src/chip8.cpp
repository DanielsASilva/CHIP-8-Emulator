#include <chip8.h>
#include <iostream>
#include <fstream>
#include <random>

#define STACK_UPPER_LIMIT 0x4F
#define PROGRAM_SPACE_START 0x200

chip8::chip8() : RAM{}, V{} { 
    
    PC = PROGRAM_SPACE_START;
    I = 0;
    SP = STACK_UPPER_LIMIT;
    DT = 0;
    ST = 0;

    opcode = 0;
    instruction = 0;
    X = 0;
    Y = 0;
    N = 0;
    NN = 0;
    NNN = 0;

    uint8_t fontset[80] =  
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
    
    for(int i = 0x50; i <= 0xA0; ++i){
        RAM[i] = fontset[i - 0x50]; 
    }

    for(int i = 0; i < 2048; ++i){
        VBUF[i] = 0xFF000000;
    }

    // Initializing PRNG

    std::mt19937 mt{ std::random_device{}() };

    std::uniform_int_distribution rand8bit{0, 255};
}

void chip8::readRAM(){

    for(int i = 0; i < 4096; ++i){

        std::cout << std::hex << static_cast<int>(RAM[i]) << " ";

        if(i % 16 == 1){
            std::cout << "\n";
        }
    }
        std::cout << std::endl;

}

// returns false if read failed
// returns true if read was succesful
bool chip8::loadROM(char ROM[]){
    std::ifstream inf{ROM, std::ios::binary};    

    if(!inf){
        std::cerr << "Couldn't open ROM\n";
        return false;
    }

    char op;

    while(inf.get(op)){
        op = static_cast<uint8_t>(op);
        RAM[PC] = op;
        PC++;
    }
    
    PC = PROGRAM_SPACE_START;
    return true;
}

void chip8::fetch(){
    opcode = RAM[PC] << 8 | RAM[PC+1];
}

void chip8::decode(){
    instruction = (opcode) >> 12;
    X = (opcode & 0x0F00) >> 8;
    Y = (opcode & 0x00F0) >> 4;
    N = opcode & 0x000F;
    NN = opcode & 0x00FF;
    NNN = opcode & 0x0FFF;
}

void chip8::execute(bool modernShift, bool keys[]){
    uint8_t flagResult;
    bool hasJumped;
    hasJumped = false;
    
    switch(instruction){
        case 0x0:
            if(NN == 0x00)
                std::cout << "0 invalid opcode: 0x0000\n";
            else if(NN == 0xE0){ // 00E0 CLEAR SCREEN 
                    for(int i = 0; i < 2048; ++i){
                        VBUF[i] = 0xFF000000;
                    }
            }
            else if(NN = 0xEE) // 00EE RETURN FROM SUBROUTINE
                PC = (RAM[SP + 1] << 8) + RAM[SP + 2]; 
                SP += 2; 
            break;
        case 0x1: // 1NNN JUMP TO NN
            PC = NNN;
            hasJumped = true;
            break;
        case 0x2: // 2NNN CALL SUBROUTINE
            // One stack address = 2 RAM addresses
            RAM[SP] = PC & 0x0FF; 
            RAM[SP - 1] = (PC & 0xFF00) >> 8;
            SP -= 2;
            PC = NNN;
            hasJumped = true;
            break;
        case 0x3: // 3XNN IF VX == NN SKIP
            if(V[X] == NN)
                PC += 2;
            break;
        case 0x4: // 4XNN IF VX != NN SKIP
            if(V[X] != NN)
                PC += 2;
            break;
        case 0x5: // 5XY0 IF VX == VY SKIP
            if(V[X] == V[Y])
                PC += 2;
            break;
        case 0x6: // 6XNN SET VX TO NN
            V[X] = NN;
            break;
        case 0x7: // 7XNN ADD NN TO VX
            V[X] += NN; break;
        case 0x8:
            switch(N){
                case 0x0: // 8XY0 SET VX TO VY
                    V[X] = V[Y];
                    break;        
                case 0x1: // 8XY1 VX |= VY
                    V[X] |= V[Y];
                    break;        
                case 0x2: // 8XY2 VX &= VY
                    V[X] &= V[Y];
                    break;        
                case 0x3: // 8XY3 VX ^= VY
                    V[X] ^= V[Y];
                    break;        
                case 0x4: // 8XY4 VX += VY
                    if(V[X] + V[Y] >= 255)
                        flagResult = 1;
                    else
                        flagResult = 0;
                    V[X] += V[Y];
                    V[0xF] = flagResult;
                    break;        
                case 0x5: // 8XY5 VX -= VY
                    if(V[X] >= V[Y])
                        flagResult = 1;
                    else
                        flagResult = 0;
                    V[X] -= V[Y];
                    V[0xF] = flagResult;
                    break;        
                case 0x6: // 8XY6 SHIFT RIGHT
                    if(!modernShift)
                        V[X] = V[Y];
                    flagResult = V[X] & 0x01;
                    V[X] = V[X] >> 1;
                    V[0xF] = flagResult;
                    break;        
                case 0x7: // 8XY7 VX = VY - VX
                    if(V[Y] >= V[X])
                        flagResult = 1;
                    else
                        flagResult = 0;
                    V[X] = V[Y] - V[X];
                    V[0xF] = flagResult;
                    break;        
                case 0xE: // 8XYE SHIFT LEFT
                    if(!modernShift)
                        V[X] = V[Y];
                    flagResult = (V[X] & 0x80) >> 7;
                    V[X] = V[X] << 1;
                    V[0xF] = flagResult;
                    break;        
                default:
                    std::cout << "8 invalid opcode: " << opcode << "\n";
                    break;
                }
            break;
        case 0x9: // 9XY0 IF X != Y SKIP
            if(V[X] != V[Y])
                PC += 2;
            break;
        case 0xA: // ANNN set I to NNN
            I = NNN;
            break;
        case 0xB: // BNNN JUMP TO NNN OFF V0
            PC = NNN + V[0];
            hasJumped = true;
            break;
        case 0xC: // CXNN RANDOM AND
            V[X] = NN & rand8bit(mt);
            break;  
        case 0xD: // DXYN DRAW AT X,Y
            {
                // Sets the X and Y coordinates
                uint8_t Xd = V[X] % 64;
                uint8_t Yd = V[Y] % 32;

                // Clears the flag register
                V[0xF] = 0;
                
                uint8_t spriteRow;
                uint8_t currentPixel;

                int pixelIndex; 
                // For N rows
                for(int i = 0; i < N; ++i){
                    // Gets the Nth byte of sprite data starting from address in I
                    spriteRow = RAM[I+i];  

                    // For each pixel on the sprite row
                    for(int j = 0; j < 8; ++j){
                        currentPixel = (spriteRow >> (7 - j) & 0x01);
                        pixelIndex = ((Yd + i) * 64) + (Xd + j);
                        // If it arrives on the right edge, stop drawing
                        if(Xd + j >= 64)
                            break;
                        else
                            // If current pixel is on and the screen pixel is also on,
                            // turn the screen pixel off and set VF to 1
                            if(currentPixel && VBUF[pixelIndex] == 0xFFFFFFFF){
                                VBUF[pixelIndex] = 0xFF000000;
                                V[0xF] = 1;
                            // If current pixel is on and the screen pixel is off,
                            // turn the screen pixel on
                            } else if(currentPixel && VBUF[pixelIndex] == 0xFF000000){
                                VBUF[pixelIndex] = 0xFFFFFFFF;
                            }
                    }

                    // If it arrives on the bottom edge, stop drawing
                    if(Yd + i >= 32)
                        break;
                }
            }
            break;  
        case 0xE:
            if(NN == 0x9E) { // EX9NN SKIP IF KEY PRESSED
                if(keys[V[X]])
                    PC += 2;
            } else if(NN == 0xA1) { // EXA1 SKIP IF KEY NOT PRESSED
                if(!keys[V[X]])
                    PC += 2;
            }
            break;  
        case 0xF:
            switch(NN){
                case 0x07: // FX07 VX = DELAY TIMER
                    V[X] = DT;
                    break;
                case 0x0A: // FX0A GET KEY
                {
                    bool keyPressed = false;

                    for(int i = 0; i <= 0xF; i++){
                        if(keys[i]) {
                            keyPressed = true;
                            V[X] = i;
                            break;
                        }
                    }

                    if(!keyPressed)
                        PC -= 2;
                    break;
                }
                case 0x15: // FX15 DELAY TIMER = VX
                    DT = V[X];
                    break;
                case 0x18: // FX18 SOUND TIMER = VX
                    ST = V[X];
                    break;
                case 0x1E: // FX1E I += VX
                    I += V[X];
                    if(I > 0x1000)
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;
                    break;
                case 0x29: // FX29 SET I TO FONT CHARACTER
                    I = 0x50 + (5 * V[X]);
                    break;
                case 0x33: // FX33 VX TO BCD
                    RAM[I] = (V[X] / 100) % 10;
                    RAM[I + 1] = (V[X] / 10) % 10;
                    RAM[I + 2] = V[X] % 10;
                    break;
                case 0x55: // FX55 STORE MEMORY
                    for(int i = 0; i <= X; i++)
                        RAM[I + i] = V[i];
                    break;
                case 0x65: // FX65 LOAD MEMORY
                    for(int i = 0; i <= X; i++)
                        V[i] = RAM[I + i];
                    break;
                default:
                    std::cout << "f invalid opcode: " << std::hex << opcode << "\n";
                    break;
            }
            break;
        default:
            std::cout << "op invalid opcode: " << std::hex << opcode << "\n";
            break;
    }

        if(!hasJumped)
            PC += 2;
}

void chip8::disassemble(){
    while(PC <= 4096){
        opcode = RAM[PC] << 8 | RAM[PC+1];
        instruction = (opcode) >> 12;
        X = (opcode & 0x0F00) >> 8;
        Y = (opcode & 0x00F0) >> 4;
        N = opcode & 0x000F;
        NN = opcode & 0x00FF;
        NNN = opcode & 0x0FFF;

        std::cout << std::hex << static_cast<int>(opcode) << " ";

        PC += 2; 

        switch(instruction){
            case 0x0:
                if(NN == 0x00)
                    std::cout << "invalid";
                else if(NN == 0xE0) 
                    std::cout << "CLEAR";
                else if(NN = 0xEE)
                    std::cout << "RETURN";
                break;
            case 0x1:
                std::cout << "JUMP " << std::hex << static_cast<int>(NNN);
                break;
            case 0x2:
                std::cout << "CALL " << std::hex << static_cast<int>(NNN);
                break;
            case 0x3:
                std::cout << "IF V" << std::hex << static_cast<int>(X) << " == " << std::hex << static_cast<int>(NN) << " SKIP";
                break;
            case 0x4:
                std::cout << "IF V" << std::hex << static_cast<int>(X) << " != " << std::hex << static_cast<int>(NN) << " SKIP";
                break;
            case 0x5:
                std::cout << "IF V" << std::hex << static_cast<int>(X) << " == V" << std::hex << static_cast<int>(Y) << " SKIP";
                break;
            case 0x6:
                std::cout << "V" << std::hex << static_cast<int>(X) << " := " << std::hex << static_cast<int>(NN);
                break;
            case 0x7:
                std::cout << "V" << std::hex << static_cast<int>(X) << " += " << std::hex << static_cast<int>(NN);
                break;
            case 0x8:
                switch(N){
                    case 0x0:
                        std::cout << "V" << std::hex << static_cast<int>(X) << " := V" << std::hex << static_cast<int>(Y);
                        break;        
                    case 0x1:
                        std::cout << "V" << std::hex << static_cast<int>(X) << " |= V" << std::hex << static_cast<int>(Y);
                        break;        
                    case 0x2:
                        std::cout << "V" << std::hex << static_cast<int>(X) << " &= V" << std::hex << static_cast<int>(Y);
                        break;        
                    case 0x3:
                        std::cout << "V" << std::hex << static_cast<int>(X) << " ^= V" << std::hex << static_cast<int>(Y);
                        break;        
                    case 0x4:
                        std::cout << "V" << std::hex << static_cast<int>(X) << " += V" << std::hex << static_cast<int>(Y);
                        break;        
                    case 0x5:
                        std::cout << "V" << std::hex << static_cast<int>(X) << " -= V" << std::hex << static_cast<int>(Y);
                        break;        
                    case 0x6:
                        std::cout << "V" << std::hex << static_cast<int>(X) << " >>= V" << std::hex << static_cast<int>(Y);
                        break;        
                    case 0x7:
                        std::cout << "V" << std::hex << static_cast<int>(X) << " =- V" << std::hex << static_cast<int>(Y);
                        break;        
                    case 0xE:
                        std::cout << "V" << std::hex << static_cast<int>(X) << " <<= V" << std::hex << static_cast<int>(Y);
                        break;        
                    default:
                        std::cout << "invalid";
                        break;
                    }
                break;
            case 0x9:
                std::cout << "IF V" << std::hex << static_cast<int>(X) << " != V" << std::hex << static_cast<int>(Y) << " SKIP";
                break;
            case 0xA:
                std::cout << "I := " << std::hex << static_cast<int>(NNN);
                break;
            case 0xB:
                std::cout << "JUMP0 " << std::hex << static_cast<int>(NNN);
                break;
            case 0xC:
                std::cout << "V" << std::hex << static_cast<int>(X) << ":= RANDOM " << std::hex << static_cast<int>(NN);
                break;  
            case 0xD:
                std::cout << "SPRITE V" << std::hex << static_cast<int>(X) << " V" << std::hex << static_cast<int>(Y) << " " << std::hex << static_cast<int>(N);
                break;  
            case 0xE:
                if(N == 0xE)
                    std::cout << "IF V" << static_cast<int>(X) << " -KEY THEN";
                else
                    std::cout << "IF V" << static_cast<int>(X) << " KEY THEN";
                break;  
            case 0xF:
                switch(NN){
                    case 0x07:
                        std::cout << "V" << static_cast<int>(X) << " DELAY";
                        break;
                    case 0x0A:
                        std::cout << "V" << static_cast<int>(X) << " KEY";
                        break;
                    case 0x15:
                        std::cout << "DELAY := V" << static_cast<int>(X); 
                        break;
                    case 0x18:
                        std::cout << "BUZZER := V" << static_cast<int>(X); 
                        break;
                    case 0x1E:
                        std::cout << "I += V" << static_cast<int>(X); 
                        break;
                    case 0x29:
                        std::cout << "I := HEX V" << static_cast<int>(X); 
                        break;
                    case 0x33:
                        std::cout << "BCD V" << static_cast<int>(X); 
                        break;
                    case 0x55:
                        std::cout << "SAVE V" << static_cast<int>(X); 
                        break;
                    case 0x65:
                        std::cout << "LOAD V" << static_cast<int>(X); 
                        break;
                    default:
                        std::cout << "invalid";
                        break;
                }
                break;
            default:
                std::cout << "invalid";
                break;
        }

        std::cout << "\n";
    } 
}

void chip8::decreaseTimers(){
    if(DT > 0)
        DT--;
    if(ST > 0)
        ST--;
}

bool chip8::isBeeping(){
    if(ST > 0)
        return true;
    else
        return false;
}
