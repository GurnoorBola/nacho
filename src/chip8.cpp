#include <chip8/chip8.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cstring>
#include <fstream>

uint8_t fonts[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
    0xF0, 0x80, 0xF0, 0x80, 0x80   // F
};

uint8_t big_fonts[] = {
    0xff, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xff,  // 0
    0x18, 0x78, 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0xff, 0xff,  // 1
    0xff, 0xff, 0x03, 0x03, 0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff,  // 2
    0xff, 0xff, 0x03, 0x03, 0xff, 0xff, 0x03, 0x03, 0xff, 0xff,  // 3
    0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xff, 0x03, 0x03, 0x03, 0x03,  // 4
    0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff, 0x03, 0x03, 0xff, 0xff,  // 5
    0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff, 0xc3, 0xc3, 0xff, 0xff,  // 6
    0xff, 0xff, 0x03, 0x03, 0x06, 0x0c, 0x18, 0x18, 0x18, 0x18,  // 7
    0xff, 0xff, 0xc3, 0xc3, 0xff, 0xff, 0xc3, 0xc3, 0xff, 0xff,  // 8
    0xff, 0xff, 0xc3, 0xc3, 0xff, 0xff, 0x03, 0x03, 0xff, 0xff,  // 9
    0x7e, 0xff, 0xc3, 0xc3, 0xc3, 0xff, 0xff, 0xc3, 0xc3, 0xc3,  // A
    0xfc, 0xfc, 0xc3, 0xc3, 0xfc, 0xfc, 0xc3, 0xc3, 0xfc, 0xfc,  // B
    0x3c, 0xff, 0xc3, 0xc0, 0xc0, 0xc0, 0xc0, 0xc3, 0xff, 0x3c,  // C
    0xfc, 0xfe, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xfe, 0xfc,  // D
    0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff,  // E
    0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff, 0xc0, 0xc0, 0xc0, 0xc0   // F
};

/*-----------------[Special Member Functions]-----------------*/
Chip8::Chip8(int mode, int speed) {
    // copy fonts to memory (0x050 - 0x09F)
    memcpy(&memory[0x50], fonts, sizeof(fonts));
    // copy big fonts to memory (0xA0 - 0x13F)
    memcpy(&memory[0xA0], big_fonts, sizeof(big_fonts));

    Chip8::mode = mode;
    Chip8::speed = speed;
};

/*-----------------[Stack]-----------------*/
void Chip8::push(uint16_t x) {
    if (SP == MAX_STACK - 1) {
        std::cerr << "Cannot push; Stack full" << std::endl;
        return;
    }
    stack[++SP] = x;
}

uint16_t Chip8::pop() {
    if (SP == -1) {
        std::cerr << "Cannot pop; Stack empty" << std::endl;
        return 0;
    }
    return stack[SP--];
}

uint16_t Chip8::peek() {
    if (SP == -1) {
        std::cerr << "Cannot peek; Stack empty" << std::endl;
        return 0;
    }
    return stack[SP];
}

/*-----------------[IO Functions]-----------------*/
// load program into memory starting from 0x200 (512)
int Chip8::loadProgram(std::string filename) {
    std::ifstream program("games/" + filename, std::ios::binary);
    if (!program.is_open()) {
        std::cerr << "Program failed to open" << std::endl;
        return 1;
    }
    program.seekg(0, std::ios::end);
    std::streampos fileSize = program.tellg();
    program.seekg(0, std::ios::beg);

    program.read(reinterpret_cast<char*>(memory + 0x200), fileSize);
    for (int i = 512; i < 512 + (int)fileSize; i++) {
        printf("%02x ", memory[i]);
        fflush(stdout);
    }
    printf("\n");
    fflush(stdout);

    PC = 0x200;
    return 0;
}

/*-----------------[Window]-----------------*/

void Chip8::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, WIDTH * 10, HEIGHT * 10);
}

void Chip8::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (window == NULL) {
        std::cerr << "GLFW window not intialized" << std::endl;
        return;
    }
    Chip8* chip8 = static_cast<Chip8*>(glfwGetWindowUserPointer(window));
    switch (action) {
        case GLFW_PRESS:
            switch (key) {
                case GLFW_KEY_ESCAPE:
                    chip8->stop = true;
                    break;

                case GLFW_KEY_1:
                    chip8->keys |= (1 << 0x1);
                    chip8->pressed = 0x1;
                    break;

                case GLFW_KEY_2:
                    chip8->keys |= (1 << 0x2);
                    chip8->pressed = 0x2;
                    break;

                case GLFW_KEY_3:
                    chip8->keys |= (1 << 0x3);
                    chip8->pressed = 0x3;
                    break;

                case GLFW_KEY_4:
                    chip8->keys |= (1 << 0xC);
                    chip8->pressed = 0xC;
                    break;

                case GLFW_KEY_Q:
                    chip8->keys |= (1 << 0x4);
                    chip8->pressed = 0x4;
                    break;

                case GLFW_KEY_W:
                    chip8->keys |= (1 << 0x5);
                    chip8->pressed = 0x5;
                    break;

                case GLFW_KEY_E:
                    chip8->keys |= (1 << 0x6);
                    chip8->pressed = 0x6;
                    break;

                case GLFW_KEY_R:
                    chip8->keys |= (1 << 0xD);
                    chip8->pressed = 0xD;
                    break;

                case GLFW_KEY_A:
                    chip8->keys |= (1 << 0x7);
                    chip8->pressed = 0x7;
                    break;

                case GLFW_KEY_S:
                    chip8->keys |= (1 << 0x8);
                    chip8->pressed = 0x8;
                    break;

                case GLFW_KEY_D:
                    chip8->keys |= (1 << 0x9);
                    chip8->pressed = 0x9;
                    break;

                case GLFW_KEY_F:
                    chip8->keys |= (1 << 0xE);
                    chip8->pressed = 0xE;
                    break;

                case GLFW_KEY_Z:
                    chip8->keys |= (1 << 0xA);
                    chip8->pressed = 0xA;
                    break;

                case GLFW_KEY_X:
                    chip8->keys |= (1 << 0x0);
                    chip8->pressed = 0x0;
                    break;

                case GLFW_KEY_C:
                    chip8->keys |= (1 << 0xB);
                    chip8->pressed = 0xB;
                    break;

                case GLFW_KEY_V:
                    chip8->keys |= (1 << 0xF);
                    chip8->pressed = 0xF;
                    break;
            }
            break;

        case GLFW_RELEASE:
            switch (key) {
                case GLFW_KEY_1:
                    chip8->keys ^= (1 << 0x1);
                    break;

                case GLFW_KEY_2:
                    chip8->keys ^= (1 << 0x2);
                    break;

                case GLFW_KEY_3:
                    chip8->keys ^= (1 << 0x3);
                    break;

                case GLFW_KEY_4:
                    chip8->keys ^= (1 << 0xC);
                    break;

                case GLFW_KEY_Q:
                    chip8->keys ^= (1 << 0x4);
                    break;

                case GLFW_KEY_W:
                    chip8->keys ^= (1 << 0x5);
                    break;

                case GLFW_KEY_E:
                    chip8->keys ^= (1 << 0x6);
                    break;

                case GLFW_KEY_R:
                    chip8->keys ^= (1 << 0xD);
                    break;

                case GLFW_KEY_A:
                    chip8->keys ^= (1 << 0x7);
                    break;

                case GLFW_KEY_S:
                    chip8->keys ^= (1 << 0x8);
                    break;

                case GLFW_KEY_D:
                    chip8->keys ^= (1 << 0x9);
                    break;

                case GLFW_KEY_F:
                    chip8->keys ^= (1 << 0xE);
                    break;

                case GLFW_KEY_Z:
                    chip8->keys ^= (1 << 0xA);
                    break;

                case GLFW_KEY_X:
                    chip8->keys ^= (1 << 0x0);
                    break;

                case GLFW_KEY_C:
                    chip8->keys ^= (1 << 0xB);
                    break;

                case GLFW_KEY_V:
                    chip8->keys ^= (1 << 0xF);
                    break;
            }
            break;
    }
}

int Chip8::initDisplay() {
    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, 0);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WIDTH * 10, HEIGHT * 10, "CHIP-8", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, Chip8::key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    glViewport(0, 0, WIDTH * 10, HEIGHT * 10);
    glfwSetFramebufferSizeCallback(window, Chip8::framebuffer_size_callback);

    // set clear color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // create shader object
    shader = Shader("shaders/shader.vs", "shaders/shader.fs");
    shader.use();

    float vertices[] = {
        // positions            // texture coords
        1.0f,  1.0f,  0.0f, 1.0f, 1.0f,  // top right
        1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom left
        -1.0f, 1.0f,  0.0f, 0.0f, 1.0f   // top left
    };

    unsigned int indices[] = {
        0, 1, 3,  // first triangle
        1, 2, 3   // second triangle
    };

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);

    // initialize texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, WIDTH, HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, screen);

    // initialize VAO
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    return 0;
}

void Chip8::emulate_cycle() {
    // ImGui_ImplOpenGL3_NewFrame();
    // ImGui_ImplGlfw_NewFrame();
    // ImGui::NewFrame();
    // ImGui::ShowDemoWindow();

    uint16_t instruction = Chip8::fetch();
    Chip8::decode(instruction);

    glClear(GL_COLOR_BUFFER_BIT);

    shader.use();

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

    // ImGui::Render();
    // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();
    return;
}

void Chip8::decrementTimers() {
    if (delay) delay -= 1;
    if (sound) sound -= 1;
}

void Chip8::terminate() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &EBO);

    // ImGui_ImplOpenGL3_Shutdown();
    // ImGui_ImplGlfw_Shutdown();
    // ImGui::DestroyContext();

    glfwTerminate();
}

/*-----------------[Main Functionality]-----------------*/

// get 2 byte instruction at PC location and increment by 2
uint16_t Chip8::fetch() {
    if (PC > MAX_MEM - 2) {
        std::cerr << "PC at end of memory; failed to fetch" << std::endl;
        return 0;
    }
    uint16_t mask = 0xFFFF;
    mask &= ((memory[PC] << 8) + (memory[PC + 1]));
    PC += 2;
    return mask;
}

// determine what to do based on instruction
void Chip8::decode(uint16_t instruction) {
    switch (instruction >> 12) {
        case 0x0: {
            switch ((instruction >> 4) & 0xF) {
                case 0xE: {
                    switch (instruction & 0xF) {
                        case 0x0: 
                            Chip8::clear();
                            break;
                        
                        case 0xE: 
                            Chip8::return_subroutine();
                            break;

                        default:
                            std::cerr << "Invalid opcode " << std::hex << instruction << std::endl;
                    }
                    break;
                }
                
                case 0xC: {
                    uint8_t val = instruction & 0xF;
                    Chip8::scroll_down_n(val);
                    break;
                }

                case 0xF: {
                    switch (instruction & 0xF){
                        case 0xB: 
                            Chip8::scroll_right_four();
                            break;
                         
                        case 0xC: 
                            Chip8::scroll_Left_four();
                            break;
                        
                        case 0xD: 
                            Chip8::exit();
                            break;
                        
                        case 0xE: 
                            Chip8::switch_lores();
                            break;
                        
                        case 0xF: 
                            Chip8::switch_hires();
                            break; 
                        
                        default:
                            std::cerr << "Invalid opcode " << std::hex << instruction << std::endl;
                    }
                    break;
                }

                default:
                    std::cerr << "Invalid opcode " << std::hex << instruction << std::endl;
            }
            break;
        }

        case 0x1: {
            uint16_t addr = instruction & 0xFFF;
            Chip8::jump(addr);
            break;
        }

        case 0x2: {
            uint16_t addr = instruction & 0xFFF;
            Chip8::start_subroutine(addr);
            break;
        }

        case 0x3: {
            uint8_t x_reg = (instruction >> 8) & 0xF;
            uint8_t val = instruction & 0xFF;
            Chip8::skip_equals(x_reg, val);
            break;
        }

        case 0x4: {
            uint8_t x_reg = (instruction >> 8) & 0xF;
            uint8_t val = instruction & 0xFF;
            Chip8::skip_not_equals(x_reg, val);
            break;
        }

        case 0x5: {
            uint8_t x_reg = (instruction >> 8) & 0xF;
            uint8_t y_reg = (instruction >> 4) & 0xF;
            Chip8::skip_reg_equals(x_reg, y_reg);
            break;
        }

        case 0x6: {
            uint8_t x_reg = (instruction >> 8) & 0xF;
            uint8_t val = instruction & 0xFF;
            Chip8::set(x_reg, val);
            break;
        }

        case 0x7: {
            uint8_t x_reg = (instruction >> 8) & 0xF;
            uint8_t val = instruction & 0xFF;
            Chip8::add(x_reg, val);
            break;
        }

        case 0x8: {
            uint8_t x_reg = (instruction >> 8) & 0xF;
            uint8_t y_reg = (instruction >> 4) & 0xF;

            switch (instruction & 0xF) {
                case 0x0:
                    Chip8::set_reg_equals(x_reg, y_reg);
                    break;

                case 0x1:
                    Chip8::set_reg_or(x_reg, y_reg);
                    break;

                case 0x2:
                    Chip8::set_reg_and(x_reg, y_reg);
                    break;

                case 0x3:
                    Chip8::set_reg_xor(x_reg, y_reg);
                    break;

                case 0x4:
                    Chip8::set_reg_sum(x_reg, y_reg);
                    break;

                case 0x5:
                    Chip8::set_reg_sub_Y(x_reg, y_reg);
                    break;

                case 0x6:
                    Chip8::set_reg_shift_right(x_reg, y_reg);
                    break;

                case 0x7:
                    Chip8::set_reg_sub_X(x_reg, y_reg);
                    break;

                case 0xE:
                    Chip8::set_reg_shift_left(x_reg, y_reg);
                    break;

                default:
                    std::cerr << "Invalid opcode " << std::hex << instruction << std::endl;
            }
            break;
        }

        case 0x9: {
            uint8_t x_reg = (instruction >> 8) & 0xF;
            uint8_t y_reg = (instruction >> 4) & 0xF;
            Chip8::skip_reg_not_equals(x_reg, y_reg);
            break;
        }

        case 0xA: {
            uint16_t addr = instruction & 0xFFF;
            Chip8::set_index(addr);
            break;
        }

        case 0xB: {
            uint16_t addr = instruction & 0xFFF;
            if (mode == SCHIP1_1 || mode == SCHIP_MODERN) {
                uint8_t x_reg = (instruction >> 8) & 0xF;
                Chip8::jump_plus_reg(addr, x_reg);
            } else {
                Chip8::jump_plus(addr);
            }
            break;
        }

        case 0xC: {
            uint8_t x_reg = (instruction >> 8) & 0xF;
            uint8_t val = instruction & 0xFF;
            Chip8::set_reg_rand(x_reg, val);
            break;
        }

        case 0xD: {
            uint8_t x_reg = (instruction >> 8) & 0xF;
            uint8_t y_reg = (instruction >> 4) & 0xF;
            uint8_t height = instruction & 0xF;
            if (height == 0x0 && mode != CHIP8){
                Chip8::display_16(x_reg, y_reg);
            } else {
                Chip8::display_8(x_reg, y_reg, height);
            }
            break;
        }

        case 0xE: {
            uint8_t x_reg = (instruction >> 8) & 0xF;
            switch (instruction & 0xFF) {
                case 0x9E:
                    Chip8::skip_key_pressed(x_reg);
                    break;

                case 0xA1:
                    Chip8::skip_key_not_pressed(x_reg);
                    break;

                default:
                    std::cerr << "Invalid opcode " << std::hex << instruction << std::endl;
            }
            break;
        }

        case 0xF: {
            uint8_t x_reg = (instruction >> 8) & 0xF;
            switch (instruction & 0xFF) {
                case 0x07:
                    Chip8::set_reg_delay(x_reg);
                    break;

                case 0x0A:
                    Chip8::set_reg_keypress(x_reg);
                    break;

                case 0x15:
                    Chip8::set_delay(x_reg);
                    break;

                case 0x18:
                    Chip8::set_sound(x_reg);
                    break;

                case 0x1E:
                    Chip8::add_index(x_reg);
                    break;

                case 0x29:
                    Chip8::set_index_font(x_reg);
                    break;

                case 0x33:
                    Chip8::set_reg_BCD(x_reg);
                    break;

                case 0x55:
                    Chip8::write_reg_mem(x_reg);
                    break;

                case 0x65:
                    Chip8::read_mem_reg(x_reg);
                    break;
                
                case 0x30:
                    Chip8::set_index_font_big(x_reg);
                    break;
                
                case 0x75:
                    Chip8::write_flags_storage(x_reg);
                    break;
                
                case 0x85:
                    Chip8::read_flags_storage(x_reg);
                    break;

                default:
                    std::cerr << "Invalid opcode " << std::hex << instruction << std::endl;
            }
            break;
        }
    }
}

/*-----------------[Opcodes]-----------------*/

//[CHIP-8]
//(00E0) clear screen
void Chip8::clear() {
    std::memset(screen, 0, WIDTH * HEIGHT * sizeof(uint8_t));
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RED, GL_UNSIGNED_BYTE, screen);
}

//(00EE) return from subroutine
void Chip8::return_subroutine() { PC = Chip8::pop(); }

//(1NNN) jump to adress NNN
void Chip8::jump(uint16_t addr) { PC = addr; }

//(2NNN) start subroutine at NNN
void Chip8::start_subroutine(uint16_t addr) {
    Chip8::push(PC);
    PC = addr;
}

//(3XNN) skip if VX == NN
void Chip8::skip_equals(uint8_t x_reg, uint8_t val) {
    if (registers[x_reg] == val) {
        PC += 2;
    }
}

//(4XNN) skip if VX != NN
void Chip8::skip_not_equals(uint8_t x_reg, uint8_t val) {
    if (registers[x_reg] != val) {
        PC += 2;
    }
}

//(5XY0) skip if VX == VY
void Chip8::skip_reg_equals(uint8_t x_reg, uint8_t y_reg) {
    if (registers[x_reg] == registers[y_reg]) {
        PC += 2;
    }
}

//(6XNN) set VX to NN
void Chip8::set(uint8_t x_reg, uint8_t val) { registers[x_reg] = val; }

//(7XNN) add NN to VX
void Chip8::add(uint8_t x_reg, uint8_t val) { registers[x_reg] += val; }

//(8XY0) set VX to value of VY
void Chip8::set_reg_equals(uint8_t x_reg, uint8_t y_reg) { registers[x_reg] = registers[y_reg]; }

//(8XY1) set VX to or of value of VX and VY
void Chip8::set_reg_or(uint8_t x_reg, uint8_t y_reg) {
    registers[x_reg] |= registers[y_reg];
    if (mode != SCHIP1_1 && mode != SCHIP_MODERN){
        registers[0xF] = 0;
    }
}

//(8XY2) set VX to and of value of VX and VY
void Chip8::set_reg_and(uint8_t x_reg, uint8_t y_reg) {
    registers[x_reg] &= registers[y_reg];
    if (mode != SCHIP1_1 && mode != SCHIP_MODERN){
        registers[0xF] = 0;
    }
}

//(8XY3) set VX to xor of value of VX and VY
void Chip8::set_reg_xor(uint8_t x_reg, uint8_t y_reg) {
    registers[x_reg] ^= registers[y_reg];
    if (mode != SCHIP1_1 && mode != SCHIP_MODERN){
        registers[0xF] = 0;
    }
}

// 8XY4 set VX to sum of value of VX and VY and set VF to 1 if overflow, 0 else
void Chip8::set_reg_sum(uint8_t x_reg, uint8_t y_reg) {
    uint8_t x = registers[x_reg];
    registers[x_reg] += registers[y_reg];
    if (registers[x_reg] < x || registers[x_reg] < registers[y_reg]) {
        registers[0xF] = 1;
    } else {
        registers[0xF] = 0;
    }
}

//(8XY5) set VX to diff of value of VX and VY and set VF to 0 if underflow, 1
// else
void Chip8::set_reg_sub_Y(uint8_t x_reg, uint8_t y_reg) {
    uint8_t underflow = 1;
    if (registers[y_reg] > registers[x_reg]) {
        underflow = 0;
    }
    registers[x_reg] -= registers[y_reg];
    registers[0xF] = underflow;
}

//(8XY6) set VX to value of VY, shift VX by a bit to right and set VF to bit
// shifted out
void Chip8::set_reg_shift_right(uint8_t x_reg, uint8_t y_reg) {
    if (mode != SCHIP1_1 && mode != SCHIP_MODERN){
        registers[x_reg] = registers[y_reg];
    }
    uint8_t out = registers[x_reg] & 1;
    registers[x_reg] >>= 1;
    registers[0xF] = out;
}

//(8XY7) set VX to value of VY - VX and set VF to 0 if underflow, 1 else
void Chip8::set_reg_sub_X(uint8_t x_reg, uint8_t y_reg) {
    uint8_t underflow = 1;
    if (registers[x_reg] > registers[y_reg]) {
        underflow = 0;
    }
    registers[x_reg] = registers[y_reg] - registers[x_reg];
    registers[0xF] = underflow;
}

//(8XYE) set VX to value of VY, shift VX by a bit to left and set VF to bit
// shifted out
void Chip8::set_reg_shift_left(uint8_t x_reg, uint8_t y_reg) {
    if (mode != SCHIP1_1 && mode != SCHIP_MODERN){
        registers[x_reg] = registers[y_reg];
    }
    uint8_t out = (registers[x_reg] >> 7) & 1;
    registers[x_reg] <<= 1;
    registers[0xF] = out;
}

//(9XY0) skip if VX != VY
void Chip8::skip_reg_not_equals(uint8_t x_reg, uint8_t y_reg) {
    if (registers[x_reg] != registers[y_reg]) {
        PC += 2;
    }
}

//(ANNN) set index to NNN
void Chip8::set_index(uint16_t addr) { I = addr; }

//(BNNN) jump to NNN + V0
void Chip8::jump_plus(uint16_t addr) { PC = addr + registers[0x0]; }

//(CXNN) set VX to random byte (bitwise AND) NN
void Chip8::set_reg_rand(uint8_t x_reg, uint8_t val) {
    uint8_t res = rand() % 256;
    registers[x_reg] = res & val;
}

//(DXYN) draw sprite pointed at by I at (V[X], V[Y]) with height N
void Chip8::display_8(uint8_t x_reg, uint8_t y_reg, uint8_t height) {
    registers[0xF] = 0;

    uint16_t sprite_index = I;

    int scale = 1;
    if (lores) {
        scale = 2;
    }
    uint8_t x = registers[x_reg] * scale;
    uint8_t y = registers[y_reg] * scale;
    x %= WIDTH;
    y %= HEIGHT;

    height *= scale;
    uint8_t width = 8 * scale;

    // loop through rows and cols of the screen and update individual screen
    // pixels
    for (int row = y; row < (y + height); row += scale) {
        if (row >= HEIGHT) {
            if (!lores) {
                registers[0xF] += ((y + height) - HEIGHT);
            }
            break;
        }

        bool collision = false;

        uint8_t sprite_row = memory[sprite_index];
        int pixel_index = 7;

        for (int col = x; col < (x + width); col += scale) {
            if (col >= WIDTH) {
                break;
            }

            uint8_t bit = (sprite_row >> pixel_index) & 1;
            pixel_index--;

            if (lores) {
                int top_left = (row * WIDTH) + col;
                int top_right = (row * WIDTH) + col + 1;
                int bot_left = ((row + 1) * WIDTH) + col;
                int bot_right = ((row + 1) * WIDTH) + col + 1;
                if ((screen[top_left] & bit) || (screen[top_right] & bit) || (screen[bot_left] & bit) ||
                    (screen[bot_right] & bit)) {
                    registers[0xF] = 1;
                }
                screen[top_left] ^= bit;
                screen[top_right] ^= bit;
                screen[bot_left] ^= bit;
                screen[bot_right] ^= bit;
            } else {
                int screen_index = (row * WIDTH) + col;
                if (screen[screen_index] & bit) {
                    collision = true;
                }
                screen[screen_index] ^= bit;
            }
        }
        if (!lores && collision) {
            registers[0xF]++;
        }
        sprite_index++;
    }
    if (mode != SCHIP_MODERN) {
        draw = true;
    }
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RED, GL_UNSIGNED_BYTE, screen);
}

//(EX9E) skip if key represented by VX's lower nibble is pressed
void Chip8::skip_key_pressed(uint8_t x_reg) {
    uint8_t key = registers[x_reg] & 0xF;
    if ((keys >> key) & 1) {
        PC += 2;
    }
}

//(EXA1) skip if key represented by VX's lower nibble is not pressed
void Chip8::skip_key_not_pressed(uint8_t x_reg) {
    uint8_t key = registers[x_reg] & 0xF;
    if (!((keys >> key) & 1)) {
        PC += 2;
    }
}

//(FX07) set VX to value of delay timer
void Chip8::set_reg_delay(uint8_t x_reg) { registers[x_reg] = delay; }

//(FX0A) wait for key press and release and set VX to that key
void Chip8::set_reg_keypress(uint8_t x_reg) {
    if (!waiting) pressed = 0xFF;
    waiting = true;
    if (pressed == 0xFF) {
        PC -= 2;
    } else {
        registers[x_reg] = pressed;
        waiting = false;
    }
}

//(FX15) set delay timer to VX
void Chip8::set_delay(uint8_t x_reg) { delay = registers[x_reg]; }

//(FX18) set sound timer to VX
void Chip8::set_sound(uint8_t x_reg) { sound = registers[x_reg]; }

//(FX1E) add VX to I
void Chip8::add_index(uint8_t x_reg) { I += registers[x_reg]; }

//(FX29) set I to memory location of character represented by lower nibble of VX
void Chip8::set_index_font(uint8_t x_reg) {
    uint8_t ch = registers[x_reg] & 0xF;
    I = (0x050 + (5 * ch));
}

//(FX33) set memory at I, I+1, I+2 to be the hundredths, tens, and ones place of
// the decimal representation of VX
void Chip8::set_reg_BCD(uint8_t x_reg) {
    int num = registers[x_reg];
    for (int offset = 2; offset >= 0; offset--) {
        memory[I + (uint16_t)offset] = (uint8_t)(num % 10);
        num /= 10;
    }
}

//(FX55) write contents of V0 to VX to memory at I (classic)
void Chip8::write_reg_mem(uint8_t x_reg) {
    // TODO make toggle for below behavior
    // classic behavior modifies I
    // modern behavior doesn't
    uint16_t addr = I;
    uint16_t* addr_ptr = &addr;
    if (mode == CHIP8) {
        addr_ptr = &I;
    }
    for (uint8_t reg = 0; reg <= x_reg; reg++) {
        memory[*addr_ptr] = registers[reg];
        *addr_ptr += 1;
    }
}

//(FX65) read contents of memory at I into V0 to VX (classic)
void Chip8::read_mem_reg(uint8_t x_reg) {
    // TODO make toggle for below behavior
    // classic behavior modifies I
    // modern behavior doesn't
    uint16_t addr = I;
    uint16_t* addr_ptr = &addr;
    if (mode == CHIP8) {
        addr_ptr = &I;
    }
    for (uint8_t reg = 0; reg <= x_reg; reg++) {
        registers[reg] = memory[*addr_ptr];
        *addr_ptr += 1;
    }
}

//[SCHIP-8-1.1]

//(00CN) scroll screen down by N pixels
void Chip8::scroll_down_n(uint8_t val) {
    // start from bottom and replace with n heigher if in bounds else set to 0
    if (lores && mode == SCHIP_MODERN){
        val *= 2;
    }
    for (int row = HEIGHT - 1; row >= 0; row--) {
        for (int col = WIDTH - 1; col >= 0; col--) {
            int index = (row * WIDTH) + col;
            if ((row - val) >= 0) {
                int replace_index = ((row - val) * WIDTH) + col;
                screen[index] = screen[replace_index];
            } else {
                screen[index] = 0;
            }
        }
    }
}

//(00FB) scroll screen right by four pixels  (SCHIP Quirk: lores scrolls half)
void Chip8::scroll_right_four() {
    uint8_t val = 4;
    if (lores && mode == SCHIP_MODERN){
        val *= 2;
    }
    // traverse right to left top to down
    for (int row = 0; row < HEIGHT; row++) {
        for (int col = WIDTH - 1; col >= 0; col--) {
            int index = (row * WIDTH) + col;
            if ((col - val) >= 0) {
                int replace_index = (row * WIDTH) + (col - val);
                screen[index] = screen[replace_index];
            } else {
                screen[index] = 0;
            }
        }
    }
}

//(00FC) scroll screen left by four pixels (SCHIP Quirk: lores scrolls half)
void Chip8::scroll_Left_four() {
    uint8_t val = 4;
    if (lores && mode == SCHIP_MODERN){
        val *= 2;
    }
    // traverse left to right top to down
    for (int row = 0; row < HEIGHT; row++) {
        for (int col = 0; col < WIDTH; col++) {
            int index = (row * WIDTH) + col;
            if ((col + val) < WIDTH) {
                int replace_index = (row * WIDTH) + (col + val);
                screen[index] = screen[replace_index];
            } else {
                screen[index] = 0;
            }
        }
    }
}

//(00FD) exit interpreter
void Chip8::exit() { terminate(); }

//(00FE) switch to lores (64x32) mode
void Chip8::switch_lores() {
    // SCHIP Quirk: original didnt clear screen
    if (mode != SCHIP1_1) {
        clear();
    }
    lores = true;
}

//(00FF) switch to hires (128x64) mode
void Chip8::switch_hires() {
    // SCHIP Quirk: original didnt clear screen
    if (mode != SCHIP1_1) {
        clear();
    }
    lores = false;
}

//(BXNN) jump to XNN + V[X] (Note: this replaces BNNN which is used for classic
// and XO chip)
void Chip8::jump_plus_reg(uint16_t addr, uint8_t x_reg) {
    uint8_t val = registers[x_reg];
    PC = addr + val;
}

//(DXY0) draw (16x16) sprite at V[X], V[Y] starting from I
void Chip8::display_16(uint8_t x_reg, uint8_t y_reg) {
    if (mode == SCHIP1_1 && lores) {
        display_8(x_reg, y_reg, 0xF);
        return;
    }

    registers[0xF] = 0;
    uint16_t sprite_index = I;
    uint8_t x = registers[x_reg] % WIDTH;
    uint8_t y = registers[y_reg] % HEIGHT;

    // loop through rows and cols of the screen and update individual screen
    // pixels
    for (int row = y; row < (y + 16); row++) {
        if (row >= HEIGHT) {
            registers[0xF] += ((y + 16) - HEIGHT);
            break;
        }

        bool collision = false;

        uint16_t sprite_row = (memory[sprite_index] << 8) + memory[sprite_index + 1];

        int pixel_index = 15;

        for (int col = x; col < (x + 16); col++) {
            if (col >= WIDTH) {
                break;
            }

            uint8_t bit = (sprite_row >> pixel_index) & 1;
            pixel_index--;

            int screen_index = (row * WIDTH) + col;
            if (screen[screen_index] & bit) {
                collision = true;
            }
            screen[screen_index] ^= bit;
        }
        if (collision) {
            registers[0xF]++;
        }
        sprite_index += 2;
    }
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RED, GL_UNSIGNED_BYTE, screen);
}

//(FX30) set I to big font (10 line) for digit in lowest nibble of V[X]
void Chip8::set_index_font_big(uint8_t x_reg) {
    uint8_t ch = registers[x_reg] & 0xF;
    I = (0x0A0 + (10 * ch));
}

//(FX75) write V[0] to V[X] in flags storage
void Chip8::write_flags_storage(uint8_t x_reg) {
    for (uint8_t i = 0; i <= x_reg; i++) {
        flags[i] = registers[i];
    }
}

//(FX85) read flags[0] to flags[X] into registers
void Chip8::read_flags_storage(uint8_t x_reg) {
    for (uint8_t i = 0; i <= x_reg; i++) {
        registers[i] = flags[i];
    }
}