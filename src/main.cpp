#include <Shaders/shader.h>
#include <chip8/chip8.h>
#include <glad.h>
#include <glfw3.h>

#include <cmath>
#include <iostream>
#include <thread>

int main() {
    Chip8 chip8 = Chip8(SCHIP_MODERN, 20);
    if (chip8.initDisplay()) {
        std::cout << "Error intializing display..." << std::endl;
        return 1;
    }

    if (chip8.initAudio()) {
        std::cout << "Error intializing audio..." << std::endl;
        return 2;
    }

    // TODO prompt for filename

    // going to have main thread which is an uncapped infinite loop that simply draw

    while (chip8.loadProgram("7-beep.ch8")) {
        std::cout << "Error loading file. Please make sure the file is in the "
                     "\"games/\" directory."
                  << std::endl;
        return 3;
    }

    // create new thread to run emulation loop
    std::thread emulate(&Chip8::emulate_loop, &chip8);
    emulate.detach();

    // render screen on main thread
    chip8.render_loop();

    // terminate if we stop rendering screen
    chip8.terminate();
    return 0;
}