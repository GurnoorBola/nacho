#include <ImGuiFileDialog.h>
#include <gui/gui.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <array>
#include <string>
#include <unordered_map>

#include "cpu/cpu.h"
#include "imgui_internal.h"

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

// global size of window
const ImVec2 maxSize = ImVec2((float)WIDTH * 10, (float)HEIGHT * 12);  // The full display area
const ImVec2 minSize = maxSize * 0.5f;                                 // Half the display area
                                                                       //
const std::unordered_map<int, std::string> system_map{
    {CHIP8, "Chip8"},
    {SCHIP1_1, "SuperChip 1.1"},
    {SCHIP_MODERN, "SuperChip Modern"},
    {XO_CHIP, "XO Chip"},
};

const std::array<int, NUM_SYSTEMS> system_list{CHIP8, SCHIP1_1, SCHIP_MODERN, XO_CHIP};

void GUI::update() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    // ImGui::ShowDemoWindow();

    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {  // action if OK
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

            int fileSize = core.loadProgram(filePathName);
            if (fileSize >= 0) {
                CPU::Config config = db.gen_config(core.hash_bin(fileSize));
                core.set_config(config);
                // reload if not default start addr
                if (config.start_address != 0x200) core.loadProgram(filePathName);
            }
        }
        // close
        ImGuiFileDialog::Instance()->Close();
    }

    if (show_config) {
        ImGui::SetNextWindowSize(minSize);
        ImGui::Begin("Config", &show_config, ImGuiWindowFlags_NoResize);

        ImGui::SeparatorText("System");

        if (ImGui::BeginCombo("Core", system_map.at(curr_config.system).c_str(), ImGuiComboFlags_None)) {
            for (int i{0}; i < NUM_SYSTEMS; i++) {
                int system_option{system_list[i]};
                const bool selected{curr_config.system == system_option};

                if (ImGui::Selectable(system_map.at(system_option).c_str(), selected)) {
                    curr_config.system = i;
                }
                if (selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        

        ImGui::InputInt("Instructions per cycle", &curr_config.speed);

        ImGui::SeparatorText("Quirks");

        if (ImGui::BeginTable("quirks", 3)) {
            ImGui::TableNextColumn();
            ImGui::Checkbox("Shift", &curr_config.quirks.shift);
            ImGui::TableNextColumn();
            ImGui::Checkbox("Memory increment by X", &curr_config.quirks.memory_increment_by_X);
            ImGui::TableNextColumn();
            ImGui::Checkbox("Leave I unchanged", &curr_config.quirks.memory_leave_I_unchanged);
            ImGui::TableNextColumn();
            ImGui::Checkbox("Wrap", &curr_config.quirks.wrap);
            ImGui::TableNextColumn();
            ImGui::Checkbox("Jump", &curr_config.quirks.jump);
            ImGui::TableNextColumn();
            ImGui::Checkbox("Vblank", &curr_config.quirks.vblank);
            ImGui::TableNextColumn();
            ImGui::Checkbox("Logic", &curr_config.quirks.logic);
            ImGui::TableNextColumn();
            ImGui::Checkbox("Draw zero", &curr_config.quirks.draw_zero);
            ImGui::TableNextColumn();
            ImGui::Checkbox("Half scroll lores", &curr_config.quirks.half_scroll_lores);
            ImGui::TableNextColumn();
            ImGui::Checkbox("Clean screen", &curr_config.quirks.clean_screen);
            ImGui::TableNextColumn();
            ImGui::Checkbox("Set collisions", &curr_config.quirks.set_collisions);
            ImGui::TableNextColumn();
            ImGui::Checkbox("Lores 8 x 16", &curr_config.quirks.lores_8x16);
            ImGui::EndTable();
        }

        ImGui::SeparatorText("Colors");
        ImGuiColorEditFlags color_flags {ImGuiColorEditFlags_NoInputs};
        if (ImGui::BeginTable("colors", 4)) {
            ImGui::TableNextColumn(); ImGui::ColorEdit3("Color 1", curr_config.colors[0].data(), color_flags);
            ImGui::TableNextColumn(); ImGui::ColorEdit3("Color 2", curr_config.colors[1].data(), color_flags);
            ImGui::TableNextColumn(); ImGui::ColorEdit3("Color 3", curr_config.colors[2].data(), color_flags);
            ImGui::TableNextColumn(); ImGui::ColorEdit3("Color 4", curr_config.colors[3].data(), color_flags);
            ImGui::TableNextColumn(); ImGui::ColorEdit3("Color 5", curr_config.colors[4].data(), color_flags);
            ImGui::TableNextColumn(); ImGui::ColorEdit3("Color 6", curr_config.colors[5].data(), color_flags);
            ImGui::TableNextColumn(); ImGui::ColorEdit3("Color 7", curr_config.colors[6].data(), color_flags);
            ImGui::TableNextColumn(); ImGui::ColorEdit3("Color 8", curr_config.colors[7].data(), color_flags);
            ImGui::TableNextColumn(); ImGui::ColorEdit3("Color 9", curr_config.colors[8].data(), color_flags);
            ImGui::TableNextColumn(); ImGui::ColorEdit3("Color 10", curr_config.colors[9].data(), color_flags);
            ImGui::TableNextColumn(); ImGui::ColorEdit3("Color 11", curr_config.colors[10].data(), color_flags);
            ImGui::TableNextColumn(); ImGui::ColorEdit3("Color 12", curr_config.colors[11].data(), color_flags);
            ImGui::TableNextColumn(); ImGui::ColorEdit3("Color 13", curr_config.colors[12].data(), color_flags);
            ImGui::TableNextColumn(); ImGui::ColorEdit3("Color 14", curr_config.colors[13].data(), color_flags);
            ImGui::TableNextColumn(); ImGui::ColorEdit3("Color 15", curr_config.colors[14].data(), color_flags);
            ImGui::TableNextColumn(); ImGui::ColorEdit3("Color 16", curr_config.colors[15].data(), color_flags);
            ImGui::EndTable();
        }

        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetStyle().ItemSpacing.y - ImGui::GetFrameHeightWithSpacing());
        if (ImGui::Button("Apply")) {
            core.set_config(curr_config);
            show_config = false;
        }
        ImGui::End();
    }

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                IGFD::FileDialogConfig config;
                config.path = "games";
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".ch8,.xo8", config);
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                // TODO: add savestate functionality
            }
            if (ImGui::MenuItem("Quit", "Esc")) {
                core.terminate();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("System")) {
            if (ImGui::BeginMenu("Mode", "Ctrl+M")) {
                if (ImGui::MenuItem("Chip8")) {
                    core.set_config(db.gen_platform_config(CHIP8));
                }
                if (ImGui::MenuItem("SCHIP 1.1")) {
                    core.set_config(db.gen_platform_config(SCHIP1_1));
                }
                if (ImGui::MenuItem("SCHIP Modern")) {
                    core.set_config(db.gen_platform_config(SCHIP_MODERN));
                }
                if (ImGui::MenuItem("XO-CHIP")) {
                    core.set_config(db.gen_platform_config(XO_CHIP));
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Config")) {
                curr_config = core.config;
                core.pause();
                show_config = true;
            }
            if (ImGui::MenuItem("Reset")) {
                core.reset();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Debugger")) {
            if (ImGui::MenuItem("Pause", "Ctrl+P")) {
                core.pause();
            };
            if (ImGui::MenuItem("Resume", "Ctrl+R")) {
                show_config = false;
                core.resume();
            };
            if (ImGui::MenuItem("Step", "Ctrl+S")) {
                core.step();
            };
            if (ImGui::MenuItem("Registers")) {
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
