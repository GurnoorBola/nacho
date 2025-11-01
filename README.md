# The (greatest) Chip8 Emulator

A Chip-8 emulator with support for Classic CHIP-8, Super CHIP-8, and XO-Chip programs. 

## Description

This project is written fully in C++ and uses GLFW + OpenGL for graphics. This is my first foray into emulation, enjoy!

## Getting Started

### Dependencies

* OpenGL 3.3 or higher
* CMake version 3.16 or higher
* Note: Tested on Windows. Linux compatibility unverified.

### Build 

* Simply run the commands below from the projects root directory: 
```
cmake -S . -B build
cmake --build build
```

* A precompiled GLFW binary is included for convenience (Windows only).
Users on other platforms or those who prefer to build from source can compile GLFW themselves and replace the included library.

### Executing program

* Place programs you want to run on the emulator in the games directory of the project. (Create if it doesn't exist)
* Use the UI to slelect your game from the list and have fun! 

## Work In Progress 

* Fully setup SCHIP and XO-chip functionality
* Make UI nicer
* Add CRT effect to graphics 

## Authors

Gurnoor Bola - [Github](https://github.com/GurnoorBola)

## License

This project is licensed under the MIT License - see the [LICENSE.md](https://github.com/GurnoorBola/Chip-8-Interpreter/blob/main/LICENSE.md) file for details

## Acknowledgments

* https://tobiasvl.github.io/blog/write-a-chip-8-emulator/ - Great high-level CHIP-8 guide
* https://chip8.gulrak.net/ - Collection of CHIP-8 opcode info
* http://devernay.free.fr/hacks/chip8/C8TECH10.HTM - Cowgod's technical reference
*  https://github.com/Timendus/chip8-test-suite - Amazing compilation of CHIP-8 testing programs 