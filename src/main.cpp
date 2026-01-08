#include <cpu/cpu.h>
#include <display/display.h>

#include <iostream>
#include <stdexcept>
#include <thread>

#define INTRO_SCREEN "happy.ch8"
#define BENCHMARK_PROG "1dcell.ch8"


int main(int argc, char* argv[]) {
    CPU cpu;
    Display display(cpu);
    
    bool bench = false;
    if (argc > 1 && std::string(argv[1]) == "--benchmark") {
        bench = true;
    }
    
    std::string program = bench ? BENCHMARK_PROG : INTRO_SCREEN;
    if (cpu.loadProgram("games/" + program) < 0) {
        throw std::runtime_error("Bootup program failed to load");
    } else {
        cpu.resume();
    }
     
    if (bench) {
        //start benchmark loop
        std::thread benchmark(&CPU::benchmark, &cpu);
        benchmark.detach();
    } else {
        // create new thread to run emulation loop
        std::thread emulate(&CPU::emulate_loop, &cpu);
        emulate.detach();
    }

    // render screen on main thread
    display.render_loop();

    // if we stop we stop rendering screen
    // cpu.terminate();
    display.terminate();
    return 0;
}
