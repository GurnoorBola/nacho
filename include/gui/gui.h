#include <cpu/cpu.h>
#include <database/database.h>
#include <GLFW/glfw3.h>

class GUI {
   public:
    GUI(CPU& cpu);

    void init_gui(GLFWwindow* window);
    void update();
    void render();

   private:
    CPU& core;
    Database db;

    CPU::Config curr_config = core.config;

    //Imgui flags
    
    bool show_config {false};
};
