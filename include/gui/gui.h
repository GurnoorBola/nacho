#include <cpu/cpu.h>
#include <database/database.h>
#include <glfw3.h>

class GUI {
   public:
    GUI(CPU& cpu);

    std::atomic<bool> color_update = false;

    void init_gui(GLFWwindow* window);
    void update();
    void render();

   private:
    CPU& core;
    Database db;
};