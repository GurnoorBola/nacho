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

    cpu.pause();
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