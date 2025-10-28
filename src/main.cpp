#include <iostream>
#include <glad.h>
#include <glfw3.h>
#include <Shaders/shader.h>
#include <cmath>
#include <chip8/chip8.h>

int main() {
    Chip8 chip8 = Chip8();
    if (chip8.initDisplay()){
        std::cout << "Error intializing display..." << std::endl;
        return 1;
    }
    
    //TODO prompt for filename

    while (chip8.loadProgram("spockpaperscissors.ch8")){
        std::cout << "Error loading file. Please make sure the file is in the \"games/\" directory." << std::endl;
        return 1;
    }

    while (1) {
        if (chip8.stop) {
            break;
        }

        chip8.emulate_cycle();
    }
    chip8.terminate();
    return 0;
}