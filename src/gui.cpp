#include <gui/gui.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

GUI::GUI(CPU& cpu) : core(cpu) {}

// provide openGL window context and type
void GUI::init_gui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
}

void GUI::update() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    // ImGui::ShowDemoWindow();

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
            };
            if (ImGui::MenuItem("Quit", "Esc")) {
                core.terminate();
            }
            if (ImGui::MenuItem("Mode", "Ctrl+M")) {
            };
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Debugger")) {
            if (ImGui::MenuItem("Pause", "Ctrl+P")) {
                core.pause();
            };
            if (ImGui::MenuItem("Resume", "Ctrl+R")) {
                core.resume();
            };
            if (ImGui::MenuItem("Step", "Ctrl+S")) {
                core.step();
            };
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void GUI::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
