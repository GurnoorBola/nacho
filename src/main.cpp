#include <Shaders/shader.h>
#include <chip8/chip8.h>
#include <glad.h>
#include <glfw3.h>

#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

int main() {
  Chip8 chip8 = Chip8(SCHIP_MODERN , 30);
  if (chip8.initDisplay()) {
    std::cout << "Error intializing display..." << std::endl;
    return 1;
  }

  // TODO prompt for filename

  while (chip8.loadProgram("8-scrolling.ch8")) {
    std::cout << "Error loading file. Please make sure the file is in the "
                 "\"games/\" directory."
              << std::endl;
    return 1;
  }

  while (1) {
    for (int i = 0; i < chip8.speed; i++) {
      if (chip8.stop || chip8.draw) {
        chip8.draw = false;
        break;
      }
      chip8.emulate_cycle();
    }
    if (chip8.stop) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
    chip8.decrementTimers();
  }
  chip8.terminate();
  return 0;
}