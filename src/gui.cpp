#include <gui/gui.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

GUI::GUI(CPU& cpu) : core(cpu), db("database") {}

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
                // std::string game = "color-scroll-test-xochip.xo8";
                std::string game = "t8nks.ch8";
                int fileSize = core.loadProgram(game);
                CPU::Config config = db.gen_config(core.hash_bin(fileSize));
                core.set_config(config);
                //reload if not default start addr
                if (config.start_address != 0x200) core.loadProgram(game);
            };
            if (ImGui::MenuItem("Quit", "Esc")) {
                core.terminate();
            }
            if (ImGui::BeginMenu("Mode", "Ctrl+M")) {
                if (ImGui::MenuItem("Chip8")){
                    core.set_config(db.gen_platform_config(CHIP8));
                };
                if (ImGui::MenuItem("SCHIP 1.1")){
                    core.set_config(db.gen_platform_config(SCHIP1_1));
                };
                if (ImGui::MenuItem("SCHIP Modern")){
                    core.set_config(db.gen_platform_config(SCHIP_MODERN));
                };
                if (ImGui::MenuItem("XO-CHIP")){
                    core.set_config(db.gen_platform_config(XO_CHIP));
                };
                ImGui::EndMenu();  
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
