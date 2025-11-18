#include <cpu/cpu.h>
#include <glfw3.h>

class GUI {
   public:
    GUI(CPU& cpu);

    void init_gui(GLFWwindow* window);
    void update();
    void render();

   private:
    CPU& core;
};