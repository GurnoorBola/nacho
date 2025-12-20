#include <cpu/cpu.h>
#include <display/display.h>

#include <cmath>
#include <iostream>
#include <thread>

#define INTRO_SCREEN "happy.ch8"

int main() {
    CPU cpu;
    Display display(cpu);

    if (cpu.loadProgram("games/" + std::string(INTRO_SCREEN)) < 0) {
        std::cerr << "Intro screen failed to load" << std::endl;
    } else {
        cpu.resume();
    }

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