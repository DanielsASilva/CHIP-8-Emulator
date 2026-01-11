# CHIP-8-Emulator
A modern CHIP-8 interpreter/emulator written in C++ using SDL3 for rendering and audio
## Dependencies
- CMake
- SDL3 (Must be installed on your system)
## Build
```bash
git clone https://github.com/DanielsASilva/CHIP-8-Emulator

cd CHIP-8-Emulator/build

cmake --build ..
```
## Usage
Run the emulator by passing a path to a ROM file as a command line argument
```bash
./chip8 <path-to-rom>
```
## Controls
The original CHIP-8 had a 16-key hexadecimal keymap. This emulator maps them to the left-hand side of your keyboard
```
1	2	3	C	=>	1	2	3	4
4	5	6	D	=>	Q	W	E	R
7	8	9	E	=>	A	S	D	F
A	0	B	F	=>	Z	X	C	V
```
To quit the emulator, just press `ESC`
