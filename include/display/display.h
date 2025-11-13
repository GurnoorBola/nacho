// #include <imgui.h>
// #include <imgui_impl_glfw.h>
// #include <imgui_impl_opengl3.h>
#include <cpu/cpu.h>
#include <glad.h>
#include <glfw3.h>
#include <shaders/shader.h>
#include <miniaudio.h>

#define DEVICE_FORMAT       ma_format_f32
#define DEVICE_CHANNELS     2
#define DEVICE_SAMPLE_RATE  48000

class Display {
public:
    Display(CPU& cpu);

    void render_loop();

    void terminate();

private:
    CPU& core;

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