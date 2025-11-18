#include <Shaders/shader.h>
#include <cpu/cpu.h>
#include <display/display.h>
#include <glad.h>
#include <glfw3.h>

#include <cmath>
#include <iostream>
#include <thread>

int main() {
    CPU cpu;
    Display display(cpu);

    // TODO prompt for filename

    // going to have main thread which is an uncapped infinite loop that simply draw

    while (cpu.loadProgram("octopeg.ch8")) {
        std::cout << "Error loading file. Please make sure the file is in the "
                     "\"games/\" directory."
                  << std::endl;
        return 3;
    }

    // update on and off pixels based on config
    display.update_config();

    // create new thread to run emulation loop
    std::thread emulate(&CPU::emulate_loop, &cpu);
    emulate.detach();

    // render screen on main thread
    display.render_loop();

    // if we stop we stop rendering screen
    // cpu.terminate();
    display.terminate();
    return 0;
}