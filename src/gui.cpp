#include <gui/gui.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuiFileDialog.h>

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

    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
        if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

            int fileSize = core.loadProgram(filePathName);
            CPU::Config config = db.gen_config(core.hash_bin(fileSize));
            core.set_config(config);
            //reload if not default start addr
            if (config.start_address != 0x200) core.loadProgram(filePathName);
        }
        // close
        ImGuiFileDialog::Instance()->Close();
    }

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                IGFD::FileDialogConfig config;
                config.path = std::getenv("USERPROFILE");
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".ch8,.xo8", config);
            };
            if (ImGui::MenuItem("Reset")) {
                core.reset();
            }
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
            if (ImGui::MenuItem("Registers")){
                core.dump_reg();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void GUI::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
