#include <cpu/cpu.h>
#include <glad.h>
#include <glfw3.h>
#include <gui/gui.h>
#include <miniaudio.h>
#include <shaders/shader.h>

#define DEVICE_FORMAT ma_format_f32
#define DEVICE_CHANNELS 2
#define DEVICE_SAMPLE_RATE 48000

#define SCALE 10
#define OFFSET 3 

class Display {
   public:
    Display(CPU& cpu);

    void render_loop();

    void update_config();

    void terminate();

   private:
    CPU& core;

    GUI gui;

    GLFWwindow* window = NULL;
    Shader shader;

    ma_device device;
    ma_waveform squareWave;

    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;

    unsigned int texture;

    void init_display();

    void init_audio();

    static void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
};