#include <chip8/chip8.h>
#include <cstring>
#include <fstream>
#include <imgui.h>
#include <chrono>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

unsigned char fonts[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};


/*-----------------[Special Member Functions]-----------------*/
Chip8::Chip8(){
    //copy fonts to memory (0x050 - 0x09F)
    memcpy(&memory[0x50], fonts, sizeof(fonts));

    //int length = WIDTH*HEIGHT;
    //for (int i=0; i < length; i++){
        //screen[i] = 255;
    //}

};


/*-----------------[Stack]-----------------*/
void Chip8::push(unsigned short x){
    if (SP == MAX_STACK-1){
        std::cerr << "Cannot push; Stack full" << std::endl;
        return;
    }
    stack[++SP] = x;
}

unsigned short Chip8::pop(){
    if (SP == -1) {
        std::cerr << "Cannot pop; Stack empty" << std::endl;
        return 0;
    }
    return stack[SP--];
}

unsigned short Chip8::peek(){
    if (SP == -1) {
        std::cerr << "Cannot peek; Stack empty" << std::endl;
        return 0;
    }
    return stack[SP];
}

/*-----------------[IO Functions]-----------------*/
//load program into memory starting from 0x200 (512)
int Chip8::loadProgram(std::string filename){
    std::ifstream program("games/" + filename, std::ios::binary);
    if (!program.is_open()){
        std::cerr << "Program failed to open" << std::endl;
        return 1;
    }
    program.seekg(0, std::ios::end);
    std::streampos fileSize = program.tellg();
    program.seekg(0, std::ios::beg);

    program.read(reinterpret_cast<char*>(memory + 0x200), fileSize);
    for (int i=512; i < 512 + (int)fileSize; i++){
        printf("%02x ", memory[i]);
        fflush(stdout);
    }
    printf("\n");
    fflush(stdout);

    PC = 0x200;
    return 0;
}



/*-----------------[Graphics]-----------------*/

void Chip8::framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, WIDTH*20, HEIGHT*20);
}

void Chip8::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
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

int Chip8::initDisplay(){
    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, 0);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WIDTH*20, HEIGHT*20, "CHIP-8", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, Chip8::key_callback);


    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
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

    glViewport(0, 0, WIDTH*20, HEIGHT*20);
    glfwSetFramebufferSizeCallback(window, Chip8::framebuffer_size_callback);

    //set clear color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //create shader object
    shader = Shader("shaders/shader.vs", "shaders/shader.fs");
    shader.use();

    float vertices[] = {
        // positions            // texture coords
        1.0f,  1.0f, 0.0f,     1.0f, 1.0f,   // top right
        1.0f, -1.0f, 0.0f,     1.0f, 0.0f,   // bottom right
        -1.0f, -1.0f, 0.0f,     0.0f, 0.0f,   // bottom left
        -1.0f,  1.0f, 0.0f,     0.0f, 1.0f    // top left 
    };

    unsigned int indices[] = {
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    }; 
    
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);
   
    //initialize texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, WIDTH, HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, screen);

    //initialize VAO
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);


    //small test display 1A
    //Chip8::set_index(0x50 + (1)*5);
    //Chip8::display(1, 1, 5);
    //Chip8::set_index(0x50 + (0xA)*5);
    //Chip8::display(7, 1, 5);

    return 0;
}

void Chip8::emulate_cycle(){
    auto start = std::chrono::high_resolution_clock::now();
    //ImGui_ImplOpenGL3_NewFrame();
    //ImGui_ImplGlfw_NewFrame();
    //ImGui::NewFrame();
    //ImGui::ShowDemoWindow();

    //decrement timers

    unsigned short instruction = Chip8::fetch();
    Chip8::decode(instruction);

    glClear(GL_COLOR_BUFFER_BIT);

    shader.use();

    //glBindTexture(GL_TEXTURE_2D, texture);


    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

    //ImGui::Render();
    //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
    last += elapsed.count();

    if (last >= 16){
        if (delay) delay = std::max(0, sound - 1*(last/16));
        if (sound) sound = std::max(0, sound - 1*(last/16));
        last %= 16;
    }
    glfwPollEvents();
    return;
}

void Chip8::terminate(){
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &EBO);

    //ImGui_ImplOpenGL3_Shutdown();
    //ImGui_ImplGlfw_Shutdown();
    //ImGui::DestroyContext();

    glfwTerminate();
}


/*-----------------[Main Functionality]-----------------*/

//get 2 byte instruction at PC location and increment by 2
unsigned short Chip8::fetch(){
    if (PC > MAX_MEM-2){
        std::cerr << "PC at end of memory; failed to fetch" << std::endl;
        return 0;
    }
    unsigned short mask = 0xFFFF;
    mask &= ((memory[PC] << 8) + (memory[PC + 1]));
    PC += 2;
    return mask;
}

//determine what to do based on instruction
void Chip8::decode(unsigned short instruction){
    switch (instruction >> 12){
        case 0x0: {
            switch (instruction & 0xFFF){
                case 0x0E0:
                    Chip8::clear();
                    break;

                case 0x0EE:
                    Chip8::return_subroutine();
                    break;

                default:
                    std::cerr << "Invalid opcode" << std::endl;
            }
            break;
        }

        case 0x1: {
            unsigned short addr = instruction & 0xFFF;
            Chip8::jump(addr);
            break;
        }

        case 0x2: {
            unsigned short addr = instruction & 0xFFF;
            Chip8::start_subroutine(addr);
            break;
        }
            
        case 0x3: {
            unsigned char x_reg = (instruction >> 8) & 0xF;
            unsigned char val = instruction & 0xFF;
            Chip8::skip_equals(x_reg, val);
            break;
        }

        case 0x4: {
            unsigned char x_reg = (instruction >> 8) & 0xF;
            unsigned char val = instruction & 0xFF;
            Chip8::skip_not_equals(x_reg, val);
            break;
        }

        case 0x5: {
            unsigned char x_reg = (instruction >> 8) & 0xF;
            unsigned char y_reg = (instruction >> 4) & 0xF;
            Chip8::skip_reg_equals(x_reg, y_reg);
            break;
        }

        case 0x6: {
            unsigned char x_reg = (instruction >> 8) & 0xF;
            unsigned char val = instruction & 0xFF; 
            Chip8::set(x_reg, val);
            break;
        }

        case 0x7: {
            unsigned char x_reg = (instruction >> 8) & 0xF;
            unsigned char val = instruction & 0xFF; 
            Chip8::add(x_reg, val);
            break;
        }

        case 0x8: {
            unsigned char x_reg = (instruction >> 8) & 0xF;
            unsigned char y_reg = (instruction >> 4) & 0xF;

            switch(instruction & 0xF){
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
                    std::cerr << "Invalid opcode" << std::endl;
            }
            break;
        }

        case 0x9: {
            unsigned char x_reg = (instruction >> 8) & 0xF;
            unsigned char y_reg = (instruction >> 4) & 0xF;
            Chip8::skip_reg_not_equals(x_reg, y_reg);
            break;
        }
            
        case 0xA: {
            unsigned short addr = instruction & 0xFFF;
            Chip8::set_index(addr);
            break;
        }

        case 0xB: {
            unsigned short addr = instruction & 0xFFF;
            Chip8::jump_plus(addr);
            break;
        }

        case 0xC: {
            unsigned char x_reg = (instruction >> 8) & 0xF;
            unsigned char val = instruction & 0xFF;
            Chip8::set_reg_rand(x_reg, val);
            break;
        }
            
        case 0xD: {
            unsigned char x_reg = (instruction >> 8) & 0xF;
            unsigned char y_reg = (instruction >> 4) & 0xF;
            unsigned char height = instruction & 0xF;
            Chip8::display(x_reg, y_reg, height);
            break;
        }

        case 0xE: {
            unsigned char x_reg = (instruction >> 8) & 0xF;
            switch (instruction & 0xFF) {
                case 0x9E:
                    Chip8::skip_key_pressed(x_reg);
                    break;
                
                case 0xA1:
                    Chip8::skip_key_not_pressed(x_reg);
                    break;
                
                default:
                    std::cerr << "Invalid opcode" << std::endl;
            }
            break;
        }

        case 0xF: {
            unsigned char x_reg = (instruction >> 8) & 0xF;
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

                default:
                    std::cerr << "Invalid opcode" << std::endl;
            }
            break;
        }
    }
}


/*-----------------[Opcodes]-----------------*/

//(00E0) clear screen
void Chip8::clear(){
    std::memset(screen, 0, WIDTH*HEIGHT*sizeof(unsigned char));
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RED, GL_UNSIGNED_BYTE, screen);
}

//(00EE) return from subroutine
void Chip8::return_subroutine(){
    PC = Chip8::pop();
}

//(1NNN) jump to adress NNN
void Chip8::jump(unsigned short addr){
    PC = addr;
}

//(2NNN) start subroutine at NNN
void Chip8::start_subroutine(unsigned short addr){
    Chip8::push(PC);
    PC = addr; 
}

 //(3XNN) skip if VX == NN
void Chip8::skip_equals(unsigned char x_reg, unsigned char val){
    if (registers[x_reg] == val) {
        PC += 2;
    }
}

//(4XNN) skip if VX != NN
void Chip8::skip_not_equals(unsigned char x_reg, unsigned char val){
    if (registers[x_reg] != val) {
        PC += 2;
    }
}

//(5XY0) skip if VX == VY
void Chip8::skip_reg_equals(unsigned char x_reg, unsigned char y_reg){
    if (registers[x_reg] == registers[y_reg]) {
        PC += 2;
    }
}

//(6XNN) set VX to NN
void Chip8::set(unsigned char x_reg, unsigned char val){
    registers[x_reg] = val;
}

//(7XNN) add NN to VX
void Chip8::add(unsigned char x_reg, unsigned char val) {
    registers[x_reg] += val;
}

//(8XY0) set VX to value of VY 
void Chip8::set_reg_equals(unsigned char x_reg, unsigned char y_reg){
    registers[x_reg] = registers[y_reg];
}

//(8XY1) set VX to or of value of VX and VY 
void Chip8::set_reg_or(unsigned char x_reg, unsigned char y_reg) {
    registers[x_reg] |= registers[y_reg];
    registers[0xF] = 0;
}

//(8XY2) set VX to and of value of VX and VY 
void Chip8::set_reg_and(unsigned char x_reg, unsigned char y_reg){
    registers[x_reg] &= registers[y_reg];
    registers[0xF] = 0;
}

//(8XY3) set VX to xor of value of VX and VY 
void Chip8::set_reg_xor(unsigned char x_reg, unsigned char y_reg){
    registers[x_reg] ^= registers[y_reg];
    registers[0xF] = 0;
}

 //8XY4 set VX to sum of value of VX and VY and set VF to 1 if overflow, 0 else
void Chip8::set_reg_sum(unsigned char x_reg, unsigned char y_reg){
    unsigned char x = registers[x_reg];
    registers[x_reg] += registers[y_reg];
    if (registers[x_reg] < x || registers[x_reg] < registers[y_reg]){
        registers[0xF] = 1;
    } else {
        registers[0xF] = 0;
    }
}

//(8XY5) set VX to diff of value of VX and VY and set VF to 0 if underflow, 1 else
void Chip8::set_reg_sub_Y(unsigned char x_reg, unsigned char y_reg){
    unsigned char underflow = 1;
    if (registers[y_reg] > registers[x_reg]){
        underflow = 0; 
    }
    registers[x_reg] -= registers[y_reg];
    registers[0xF] = underflow;
}

//(8XY6) set VX to value of VY, shift VX by a bit to right and set VF to bit shifted out
void Chip8::set_reg_shift_right(unsigned char x_reg, unsigned char y_reg){
    registers[x_reg] = registers[y_reg];
    unsigned char out = registers[x_reg] & 1;
    registers[x_reg] >>= 1;
    registers[0xF] = out;
}

//(8XY7) set VX to value of VY - VX and set VF to 0 if underflow, 1 else 
void Chip8::set_reg_sub_X(unsigned char x_reg, unsigned char y_reg){
    unsigned char underflow = 1;
    if (registers[x_reg] > registers[y_reg]){
        underflow = 0;
    }
    registers[x_reg] = registers[y_reg] - registers[x_reg];
    registers[0xF] = underflow;
}

//(8XYE) set VX to value of VY, shift VX by a bit to left and set VF to bit shifted out
void Chip8::set_reg_shift_left(unsigned char x_reg, unsigned char y_reg){
    registers[x_reg] = registers[y_reg];
    unsigned char out = (registers[x_reg] >> 7) & 1;
    registers[x_reg] <<= 1;
    registers[0xF] = out;
}

//(9XY0) skip if VX != VY
void Chip8::skip_reg_not_equals(unsigned char x_reg, unsigned char y_reg){
    if (registers[x_reg] != registers[y_reg]){
        PC += 2;
    }
}

//(ANNN) set index to NNN
void Chip8::set_index(unsigned short addr){
    I = addr;
}

//(BNNN) jump to NNN + V0
void Chip8::jump_plus(unsigned short addr){
    PC = addr + registers[0x0];
}

//(CXNN) set VX to random byte (bitwise AND) NN
void Chip8::set_reg_rand(unsigned char x_reg, unsigned char val){
    unsigned char res = rand() % 256;
    registers[x_reg] = res & val;
}

//(DXYN) draw sprite pointed at by I at (V[X], V[Y]) with height N 
void Chip8::display(unsigned char x_reg, unsigned char y_reg, unsigned char height){
    registers[0xF] = 0;
    unsigned short sprite_index = I;
    unsigned char x = registers[x_reg] % WIDTH;
    unsigned char y = registers[y_reg] % HEIGHT;

    //loop through rows and cols of the screen and update individual screen pixels
    for (int row = y; row < (y + height); row++){
        if (row >= HEIGHT) { break; }
        
        unsigned char sprite_row = memory[sprite_index];
        int pixel = 7;

        for (int col = x; col < (x + 8); col++){
            if (col >= WIDTH){ break; }

            unsigned char bit = -((sprite_row >> pixel) & 1);
            pixel--;

            int screen_index = (row*WIDTH) + col;
            if (screen[screen_index] & bit) { registers[0xF] = 1; }
            screen[screen_index] ^= bit;
        }
        sprite_index++;
    }

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RED, GL_UNSIGNED_BYTE, screen);
}

//(EX9E) skip if key represented by VX's lower nibble is pressed
void Chip8::skip_key_pressed(unsigned char x_reg){
    unsigned char key = registers[x_reg] & 0xF;
    if ((keys>> key) & 1){
        PC += 2;
    }
}

//(EXA1) skip if key represented by VX's lower nibble is not pressed
void Chip8::skip_key_not_pressed(unsigned char x_reg){
    unsigned char key = registers[x_reg] & 0xF;
    if (!((keys>> key) & 1)){
        PC += 2;
    }
}

//(FX07) set VX to value of delay timer
void Chip8::set_reg_delay(unsigned char x_reg){
    registers[x_reg] = delay;
}

//(FX0A) wait for key press and release and set VX to that key
void Chip8::set_reg_keypress(unsigned char x_reg){
    if (!waiting) pressed = 0xFF;
    waiting = true;
    if (pressed == 0xFF){
        PC -= 2;
    } else {
        registers[x_reg] = pressed;
    }
}

//(FX15) set delay timer to VX
void Chip8::set_delay(unsigned char x_reg){
    delay = registers[x_reg];
}

//(FX18) set sound timer to VX
void Chip8::set_sound(unsigned char x_reg){
    sound = registers[x_reg];
}

//(FX1E) add VX to I
void Chip8::add_index(unsigned char x_reg){
    I += registers[x_reg];
}

//(FX29) set I to memory location of character represented by lower nibble of VX
void Chip8::set_index_font(unsigned char x_reg){
    unsigned char ch = registers[x_reg] & 0xF;
    I = (0x050+ (5*ch));
    std::cout << std::hex << (int)memory[I] << std::endl;
}

//(FX33) set memory at I, I+1, I+2 to be the hundredths, tens, and ones place of the decimal representation of VX
void Chip8::set_reg_BCD(unsigned char x_reg){
    int num = registers[x_reg];
    for (int offset=2; offset >= 0; offset--){
        memory[I+(unsigned short)offset] = (unsigned char)(num%10);
        num /= 10;
    }
}

//(FX55) write contents of V0 to VX to memory at I (classic)
void Chip8::write_reg_mem(unsigned char x_reg){
    //TODO make toggle for below behavior
    //classic behavior modifies I
    //modern behavior doesn't 
    unsigned short &addr = I;
    for (unsigned char reg=0; reg <= x_reg; reg++){
        memory[I] = registers[reg];
        addr += 1;
    }
}

//(FX65) read contents of memory at I into V0 to VX (classic) 
void Chip8::read_mem_reg(unsigned char x_reg){
    //TODO make toggle for below behavior
    //classic behavior modifies I
    //modern behavior doesn't 
    unsigned short &addr = I;
    for (unsigned char reg=0; reg <= x_reg; reg++){
        registers[reg] = memory[I]; 
        addr += 1;
    }
} 