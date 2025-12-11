#include <cpu/cpu.h>
#include <openssl/sha.h>

#include <cassert>
#include <chrono>
#include <cstring>
#include <fstream>
#include <json.hpp>
#include <mutex>
#include <thread>
#include <unordered_set>

using json = nlohmann::json;

const uint8_t fonts[] = {
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

const uint8_t big_fonts[] = {
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
CPU::CPU() {
    // copy fonts to memory (0x050 - 0x09F)
    memcpy(&memory[0x50], fonts, sizeof(fonts));
    // copy big fonts to memory (0xA0 - 0x13F)
    memcpy(&memory[0xA0], big_fonts, sizeof(big_fonts));
};

/*-----------------[Stack]-----------------*/
void CPU::push(uint16_t x) {
    if (SP == MAX_STACK - 1) {
        std::cerr << "Cannot push; Stack full" << std::endl;
        return;
    }
    stack[++SP] = x;
}

uint16_t CPU::pop() {
    if (SP == -1) {
        std::cerr << "Cannot pop; Stack empty" << std::endl;
        return 0;
    }
    return stack[SP--];
}

uint16_t CPU::peek() {
    if (SP == -1) {
        std::cerr << "Cannot peek; Stack empty" << std::endl;
        return 0;
    }
    return stack[SP];
}

/*-----------------[IO Functions]-----------------*/

// load program into memory starting from 0x200 (512)
int CPU::loadProgram(std::string filepath) {
    reset();
    std::cout << filepath << std::endl;
    std::ifstream program(filepath, std::ios::binary);
    if (!program.is_open()) {
        std::cerr << "Invalid file" << std::endl;
        return -1;
    }

    program.seekg(0, std::ios::end);
    std::streampos fileSize = program.tellg();
    program.seekg(0, std::ios::beg);

    if (config.start_address > MAX_MEM){
        std::cerr << "Invalid program start address" << std::endl;
        return -2;
    }

    if (fileSize > MAX_MEM - config.start_address) {
        std::cerr << "File size exceeds max program size" << std::endl;
        return -3;
    }

    program.read(reinterpret_cast<char*>(memory + config.start_address), fileSize);

    for (int i = config.start_address; i < config.start_address + (int)fileSize; i++) {
        printf("%02x ", memory[i]);
        fflush(stdout);
    }
    printf("\n");
    fflush(stdout);

    PC = config.start_address;
    return fileSize;
}

std::string CPU::hash_bin(int fileSize){
    // compute hash
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(memory + config.start_address, fileSize, hash);

    std::ostringstream oss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return oss.str();
}

/*-----------------[Access Functions]-----------------*/

void CPU::press_key(uint8_t key) {
    std::lock_guard<std::mutex> lock(key_mtx);
    keys |= (1 << key);
    pressed |= (1 << key);
}

void CPU::release_key(uint8_t key) {
    std::lock_guard<std::mutex> lock(key_mtx);
    keys ^= (1 << key);
    released = key;
}

uint8_t* CPU::get_screen() {
    std::lock_guard<std::mutex> lock(screen_mtx);
    return screen;
}

uint8_t* CPU::check_screen() {
    std::lock_guard<std::mutex> lock(screen_mtx);
    if (screen_update) {
        screen_update = false;
        return screen;
    }
    return NULL;
}

bool CPU::check_stop() {
    return stop;
}

bool CPU::check_color() {
    if (color_update) {
        color_update = false;
        return true;
    }
    return false;
}

int CPU::check_should_beep() {
    std::lock_guard<std::mutex> lock(sound_mtx);
    if (!beep && sound > 0) {
        beep = true;
        return 1;
    }
    if (beep && sound == 0) {
        beep = false;
        return 2;
    }
    return 0;
}

void CPU::set_config(Config config) {
    pause();
    CPU::config = config;    
    color_update = true;
}

void CPU::reset() {
    pause();
    PC = I = config.start_address;
    SP = -1; 
    delay = sound = 0;
    waiting = false;
    lores = true;
    beep = false;
    bit_plane = 0b01;
    pressed = NO_PRESS;
    released = NO_RELEASE;
    for (uint8_t i=0; i < 16; i++) {
        registers[i] = flags[i] = 0;
    }
    std::memset(screen, 0, WIDTH * HEIGHT * sizeof(uint8_t));
    screen_update = true;
}

void CPU::dump_reg(){
    std::cout << "<------------[Reigsters]------------>" << std::endl;
    std::cout << std::hex << "I: " << I << std::endl;
    for (int i=0; i < 16; i++) {
        std::cout << std::hex << "V"<< i  << ": " << int(registers[i]) << std::endl;
    }
    std::cout << "<----------------------------------->" << std::endl;
}

/*-----------------[Main Functionality]-----------------*/

// Do one fetch-decode cycle
void CPU::emulate_cycle() {
    uint16_t instruction = CPU::fetch();
    CPU::decode(instruction);
    if (paused) {
        std::cout << std::hex << "Executing: " << instruction << std::endl;
    }
}

// Start emulation loop running at speed instructions per cycle
void CPU::emulate_loop() {
    while (1) {
        if (!paused) {
            for (int i = 0; i < config.speed; i++) {
                if (stop) {
                    break;
                }
                if (draw) {
                    draw = false;
                    break;
                }
                emulate_cycle();
            }
        }
        if (stop) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        decrementTimers();
    }
}

// decrease sound and dealy timer by 1
void CPU::decrementTimers() {
    std::lock_guard<std::mutex> lock(sound_mtx);
    if (delay) delay -= 1;
    if (sound) sound -= 1;
}

//pause fetch decode loop
void CPU::pause() {
    paused = true;
}

//resume fetch decode loop
void CPU::resume() {
    paused = false;
}

//run one fetch decode cycle
void CPU::step() {
    if (!paused) pause();
    emulate_cycle();
}

// set stop flag to on
void CPU::terminate() {
    stop = true;
}

// get 2 byte instruction at PC location and increment by 2
uint16_t CPU::fetch() {
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
void CPU::decode(uint16_t instruction) {
    switch (instruction >> 12) {
        case 0x0: {
            switch ((instruction >> 4) & 0xF) {
                case 0xE: {
                    switch (instruction & 0xF) {
                        case 0x0:
                            CPU::clear();
                            break;

                        case 0xE:
                            CPU::return_subroutine();
                            break;

                        default:
                            std::cerr << "Invalid opcode " << std::hex << instruction << std::endl;
                    }
                    break;
                }

                case 0xC: {
                    uint8_t val = instruction & 0xF;
                    CPU::scroll_down_n(val);
                    break;
                }

                case 0xD: {
                    uint8_t val = instruction & 0xF;
                    CPU::scroll_up_n(val);
                    break;
                }

                case 0xF: {
                    switch (instruction & 0xF) {
                        case 0xB:
                            CPU::scroll_right_four();
                            break;

                        case 0xC:
                            CPU::scroll_Left_four();
                            break;

                        case 0xD:
                            CPU::exit();
                            break;

                        case 0xE:
                            CPU::switch_lores();
                            break;

                        case 0xF:
                            CPU::switch_hires();
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
            CPU::jump(addr);
            break;
        }

        case 0x2: {
            uint16_t addr = instruction & 0xFFF;
            CPU::start_subroutine(addr);
            break;
        }

        case 0x3: {
            uint8_t x_reg = (instruction >> 8) & 0xF;
            uint8_t val = instruction & 0xFF;
            CPU::skip_equals(x_reg, val);
            break;
        }

        case 0x4: {
            uint8_t x_reg = (instruction >> 8) & 0xF;
            uint8_t val = instruction & 0xFF;
            CPU::skip_not_equals(x_reg, val);
            break;
        }

        case 0x5: {
            uint8_t x_reg = (instruction >> 8) & 0xF;
            uint8_t y_reg = (instruction >> 4) & 0xF;
            switch (instruction & 0xF) {
                case 0x0:
                    CPU::skip_reg_equals(x_reg, y_reg);
                    break;
                
                case 0x2:
                    CPU::write_reg_mem_range(x_reg, y_reg);
                    break;
                
                case 0x3:
                    CPU::read_reg_mem_range(x_reg, y_reg);
                    break;
            }
            break;
        }

        case 0x6: {
            uint8_t x_reg = (instruction >> 8) & 0xF;
            uint8_t val = instruction & 0xFF;
            CPU::set(x_reg, val);
            break;
        }

        case 0x7: {
            uint8_t x_reg = (instruction >> 8) & 0xF;
            uint8_t val = instruction & 0xFF;
            CPU::add(x_reg, val);
            break;
        }

        case 0x8: {
            uint8_t x_reg = (instruction >> 8) & 0xF;
            uint8_t y_reg = (instruction >> 4) & 0xF;

            switch (instruction & 0xF) {
                case 0x0:
                    CPU::set_reg_equals(x_reg, y_reg);
                    break;

                case 0x1:
                    CPU::set_reg_or(x_reg, y_reg);
                    break;

                case 0x2:
                    CPU::set_reg_and(x_reg, y_reg);
                    break;

                case 0x3:
                    CPU::set_reg_xor(x_reg, y_reg);
                    break;

                case 0x4:
                    CPU::set_reg_sum(x_reg, y_reg);
                    break;

                case 0x5:
                    CPU::set_reg_sub_Y(x_reg, y_reg);
                    break;

                case 0x6:
                    CPU::set_reg_shift_right(x_reg, y_reg);
                    break;

                case 0x7:
                    CPU::set_reg_sub_X(x_reg, y_reg);
                    break;

                case 0xE:
                    CPU::set_reg_shift_left(x_reg, y_reg);
                    break;

                default:
                    std::cerr << "Invalid opcode " << std::hex << instruction << std::endl;
            }
            break;
        }

        case 0x9: {
            uint8_t x_reg = (instruction >> 8) & 0xF;
            uint8_t y_reg = (instruction >> 4) & 0xF;
            CPU::skip_reg_not_equals(x_reg, y_reg);
            break;
        }

        case 0xA: {
            uint16_t addr = instruction & 0xFFF;
            CPU::set_index(addr);
            break;
        }

        case 0xB: {
            uint16_t addr = instruction & 0xFFF;
            if (config.quirks.jump) {
                uint8_t x_reg = (instruction >> 8) & 0xF;
                CPU::jump_plus_reg(addr, x_reg);
            } else {
                CPU::jump_plus(addr);
            }
            break;
        }

        case 0xC: {
            uint8_t x_reg = (instruction >> 8) & 0xF;
            uint8_t val = instruction & 0xFF;
            CPU::set_reg_rand(x_reg, val);
            break;
        }

        case 0xD: {
            uint8_t x_reg = (instruction >> 8) & 0xF;
            uint8_t y_reg = (instruction >> 4) & 0xF;
            uint8_t height = instruction & 0xF;
            uint8_t width = 8;
            registers[0xF] = 0;
            //exit early if we are a system that is able to draw 0 height sprite
            if (height == 0) {
                if (config.quirks.draw_zero) {
                    if (config.quirks.vblank) draw = true;
                    return;
                }
                if (!(lores && config.quirks.lores_8x16)){
                    width = 16;
                }
                height = 16;
            }

            uint16_t mem_index = I;
            //for every bit in bit_plane (4) we check if the bit is on and draw with that plane if it is
            for (int i = 0; i < 4; i++) {
                uint8_t plane = bit_plane & (1 << i);
                if (plane) {
                    display(mem_index, plane, x_reg, y_reg, width, height);
                    mem_index += height * (width == 16 ? 2 : 1);
                }
            }

            // if (height == 0x0 && !config.quirks.draw_zero) {
            //     CPU::display_16(x_reg, y_reg);
            // } else {
            //     CPU::display_8(x_reg, y_reg, height);
            // }
            break;
        }

        case 0xE: {
            uint8_t x_reg = (instruction >> 8) & 0xF;
            switch (instruction & 0xFF) {
                case 0x9E:
                    CPU::skip_key_pressed(x_reg);
                    break;

                case 0xA1:
                    CPU::skip_key_not_pressed(x_reg);
                    break;

                default:
                    std::cerr << "Invalid opcode " << std::hex << instruction << std::endl;
            }
            break;
        }

        case 0xF: {
            uint8_t x_reg = (instruction >> 8) & 0xF;
            switch (instruction & 0xFF) {
                case 0x00:
                    CPU::set_index_long();
                    break;
                
                case 0x01:
                    CPU::select_plane(x_reg);
                    break;
                
                case 0x02:
                    CPU::set_waveform();
                    break;

                case 0x07:
                    CPU::set_reg_delay(x_reg);
                    break;

                case 0x0A:
                    CPU::set_reg_keypress(x_reg);
                    break;

                case 0x15:
                    CPU::set_delay(x_reg);
                    break;

                case 0x18:
                    CPU::set_sound(x_reg);
                    break;

                case 0x1E:
                    CPU::add_index(x_reg);
                    break;

                case 0x29:
                    CPU::set_index_font(x_reg);
                    break;

                case 0x33:
                    CPU::set_reg_BCD(x_reg);
                    break;

                case 0x3A:
                    CPU::set_pitch(x_reg);
                    break;

                case 0x55:
                    CPU::write_reg_mem(x_reg);
                    break;

                case 0x65:
                    CPU::read_mem_reg(x_reg);
                    break;

                case 0x30:
                    CPU::set_index_font_big(x_reg);
                    break;

                case 0x75:
                    CPU::write_flags_storage(x_reg);
                    break;

                case 0x85:
                    CPU::read_flags_storage(x_reg);
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
void CPU::clear() {
    std::lock_guard<std::mutex> lock(screen_mtx);
    for (int r=0; r < HEIGHT; r++) {
        for (int c=0; c < WIDTH; c++) {
            int screen_index = (r * WIDTH) + c;
            screen[screen_index] &= ~bit_plane;
        }
    }
    screen_update = true;
}

//(00EE) return from subroutine
void CPU::return_subroutine() {
    PC = CPU::pop();
}

//(1NNN) jump to adress NNN
void CPU::jump(uint16_t addr) {
    PC = addr;
}

//(2NNN) start subroutine at NNN
void CPU::start_subroutine(uint16_t addr) {
    CPU::push(PC);
    PC = addr;
}

//(3XNN) skip if VX == NN
void CPU::skip_equals(uint8_t x_reg, uint8_t val) {
    if (registers[x_reg] == val) {
        uint16_t opcode = fetch();
        if (opcode == 0xF000){
            fetch();
        }
    }
}

//(4XNN) skip if VX != NN
void CPU::skip_not_equals(uint8_t x_reg, uint8_t val) {
    if (registers[x_reg] != val) {
        uint16_t opcode = fetch();
        if (opcode == 0xF000){
            fetch();
        }
    }
}

//(5XY0) skip if VX == VY
void CPU::skip_reg_equals(uint8_t x_reg, uint8_t y_reg) {
    if (registers[x_reg] == registers[y_reg]) {
        uint16_t opcode = fetch();
        if (opcode == 0xF000){
            fetch();
        }
    }
}

//(6XNN) set VX to NN
void CPU::set(uint8_t x_reg, uint8_t val) {
    registers[x_reg] = val;
}

//(7XNN) add NN to VX
void CPU::add(uint8_t x_reg, uint8_t val) {
    registers[x_reg] += val;
}

//(8XY0) set VX to value of VY
void CPU::set_reg_equals(uint8_t x_reg, uint8_t y_reg) {
    registers[x_reg] = registers[y_reg];
}

//(8XY1) set VX to or of value of VX and VY
void CPU::set_reg_or(uint8_t x_reg, uint8_t y_reg) {
    registers[x_reg] |= registers[y_reg];
    if (config.quirks.logic) {
        registers[0xF] = 0;
    }
}

//(8XY2) set VX to and of value of VX and VY
void CPU::set_reg_and(uint8_t x_reg, uint8_t y_reg) {
    registers[x_reg] &= registers[y_reg];
    if (config.quirks.logic) {
        registers[0xF] = 0;
    }
}

//(8XY3) set VX to xor of value of VX and VY
void CPU::set_reg_xor(uint8_t x_reg, uint8_t y_reg) {
    registers[x_reg] ^= registers[y_reg];
    if (config.quirks.logic) {
        registers[0xF] = 0;
    }
}

// 8XY4 set VX to sum of value of VX and VY and set VF to 1 if overflow, 0 else
void CPU::set_reg_sum(uint8_t x_reg, uint8_t y_reg) {
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
void CPU::set_reg_sub_Y(uint8_t x_reg, uint8_t y_reg) {
    uint8_t underflow = 1;
    if (registers[y_reg] > registers[x_reg]) {
        underflow = 0;
    }
    registers[x_reg] -= registers[y_reg];
    registers[0xF] = underflow;
}

//(8XY6) set VX to value of VY, shift VX by a bit to right and set VF to bit
// shifted out
void CPU::set_reg_shift_right(uint8_t x_reg, uint8_t y_reg) {
    if (!config.quirks.shift) {
        registers[x_reg] = registers[y_reg];
    }
    uint8_t out = registers[x_reg] & 1;
    registers[x_reg] >>= 1;
    registers[0xF] = out;
}

//(8XY7) set VX to value of VY - VX and set VF to 0 if underflow, 1 else
void CPU::set_reg_sub_X(uint8_t x_reg, uint8_t y_reg) {
    uint8_t underflow = 1;
    if (registers[x_reg] > registers[y_reg]) {
        underflow = 0;
    }
    registers[x_reg] = registers[y_reg] - registers[x_reg];
    registers[0xF] = underflow;
}

//(8XYE) set VX to value of VY, shift VX by a bit to left and set VF to bit
// shifted out
void CPU::set_reg_shift_left(uint8_t x_reg, uint8_t y_reg) {
    if (!config.quirks.shift) {
        registers[x_reg] = registers[y_reg];
    }
    uint8_t out = (registers[x_reg] >> 7) & 1;
    registers[x_reg] <<= 1;
    registers[0xF] = out;
}

//(9XY0) skip if VX != VY
void CPU::skip_reg_not_equals(uint8_t x_reg, uint8_t y_reg) {
    if (registers[x_reg] != registers[y_reg]) {
        uint16_t opcode = fetch();
        if (opcode == 0xF000){
            fetch();
        }
    }
}

//(ANNN) set index to NNN
void CPU::set_index(uint16_t addr) {
    I = addr;
}

//(BNNN) jump to NNN + V0
void CPU::jump_plus(uint16_t addr) {
    PC = addr + registers[0x0];
}

//(CXNN) set VX to random byte (bitwise AND) NN
void CPU::set_reg_rand(uint8_t x_reg, uint8_t val) {
    uint8_t res = rand() % 256;
    registers[x_reg] = res & val;
}


//loop through the first four bits of bit_plane and draw with that plane if there is a 1 there
//TODO (DXYN) new version returns last memory index it drew from 
void CPU::display(uint16_t mem_index, uint8_t plane, uint8_t x_reg, uint8_t y_reg, uint8_t width, uint8_t height) {
    std::lock_guard<std::mutex> lock(screen_mtx);
    int scale = lores ? 2 : 1; //multiply everything by two if we in lores

    width *= scale;
    height *= scale;

    uint8_t x = (registers[x_reg] * scale) % WIDTH;
    uint8_t y = (registers[y_reg] * scale) % HEIGHT;

    for (int r = y; r < (y + height); r += scale) {
        int row = r;

        //we went off of screen
        if (row >= HEIGHT) {
            //if we are on a system that sets number of collisions set it
            if (config.quirks.set_collisions && !lores) {
                registers[0xF] += ((y + height) - HEIGHT);
            }
            //if we dont wrap we are done 
            if (!config.quirks.wrap) {
                break;
            }
            //otherwise wrap around and continue
            row = r % HEIGHT;
        }

        //flag for if we got a collision in this row
        bool collision = false;

        //index to select individual bits from the sprite
        int sprite_row_index;

        //if our width is 16 * scale we extract two bytes otherwise we just extract one 
        uint16_t sprite_row;
        if (width == 16 * scale) {
            sprite_row = (memory[mem_index] << 8) + memory[mem_index + 1];
            mem_index += 2;
            sprite_row_index = 15;
        } else {
            sprite_row = memory[mem_index];
            mem_index += 1;
            sprite_row_index = 7;
        }

        for (int c = x; c < (x + width); c += scale) {
            int col = c;
            if (c >= WIDTH) {
                if (!config.quirks.wrap) {
                    break;
                }
                col = c % WIDTH;
            }
            
            uint8_t bit = (sprite_row >> sprite_row_index) & 1;
            sprite_row_index -= 1;
            //if bit is 0 we can skip there is nothing to draw
            if (bit == 0) continue;
            uint8_t draw_mask = plane;

            //if we are in lores mode draw a 2x2 otherwise draw pixel by pixel
            if (lores) {
                int top_left = (row * WIDTH) + col;
                int top_right = (row * WIDTH) + col + 1;
                int bot_left = ((row + 1) * WIDTH) + col;
                int bot_right = ((row + 1) * WIDTH) + col + 1;
                if ((screen[top_left] & draw_mask) || (screen[top_right] & draw_mask) ||
                    (screen[bot_left] & draw_mask) || (screen[bot_right] & draw_mask)) {
                    collision = true;
                }
                screen[top_left] ^= draw_mask;
                screen[top_right] ^= draw_mask;
                screen[bot_left] ^= draw_mask;
                screen[bot_right] ^= draw_mask;
            } else {
                int screen_index = (row * WIDTH) + col;
                if (screen[screen_index] & draw_mask) {
                    collision = true;
                }
                screen[screen_index] ^= draw_mask;
            }
        }
        //if there is a collision either increase the vf register by 1 if we are on a system that counts them
        //otherwise just set it to 1
        if (collision) {
            if (config.quirks.set_collisions && !lores) {
                registers[0xF] += 1;
            } else {
                registers[0xF] = 1;
            }
        }
    }
    if (lores && config.quirks.vblank) {
        draw = true;
    }
    screen_update = true;
}

//(EX9E) skip if key represented by VX's lower nibble is pressed
void CPU::skip_key_pressed(uint8_t x_reg) {
    std::lock_guard<std::mutex> lock(key_mtx);
    uint8_t key = registers[x_reg] & 0xF;
    if ((keys >> key) & 1) {
        uint16_t opcode = fetch();
        if (opcode == 0xF000){
            fetch();
        }
    }
}

//(EXA1) skip if key represented by VX's lower nibble is not pressed
void CPU::skip_key_not_pressed(uint8_t x_reg) {
    std::lock_guard<std::mutex> lock(key_mtx);
    uint8_t key = registers[x_reg] & 0xF;
    if (!((keys >> key) & 1)) {
        uint16_t opcode = fetch();
        if (opcode == 0xF000){
            fetch();
        }
    }
}

//(FX07) set VX to value of delay timer
void CPU::set_reg_delay(uint8_t x_reg) {
    registers[x_reg] = delay;
}

//(FX0A) wait for key press and release and set VX to that key
void CPU::set_reg_keypress(uint8_t x_reg) {
    std::lock_guard<std::mutex> lock(key_mtx);
    if (!waiting) {
        pressed = NO_PRESS;
        released = NO_RELEASE;
    }
    waiting = true;
    if (pressed == NO_PRESS || released == NO_RELEASE || !(pressed & (1 << released))) {
        PC -= 2;
    } else {
        registers[x_reg] = released;
        pressed = NO_PRESS;
        released = NO_RELEASE;
        waiting = false;
    }
}

//(FX15) set delay timer to VX
void CPU::set_delay(uint8_t x_reg) {
    delay = registers[x_reg];
}

//(FX18) set sound timer to VX
void CPU::set_sound(uint8_t x_reg) {
    std::lock_guard<std::mutex> lock(sound_mtx);
    sound = registers[x_reg];
}

//(FX1E) add VX to I
void CPU::add_index(uint8_t x_reg) {
    I += registers[x_reg];
}

//(FX29) set I to memory location of character represented by lower nibble of VX
void CPU::set_index_font(uint8_t x_reg) {
    uint8_t ch = registers[x_reg] & 0xF;
    I = (0x050 + (5 * ch));
}

//(FX33) set memory at I, I+1, I+2 to be the hundredths, tens, and ones place of
// the decimal representation of VX
void CPU::set_reg_BCD(uint8_t x_reg) {
    int num = registers[x_reg];
    for (int offset = 2; offset >= 0; offset--) {
        memory[I + (uint16_t)offset] = (uint8_t)(num % 10);
        num /= 10;
    }
}

//(FX55) write contents of V0 to VX to memory at I (classic)
void CPU::write_reg_mem(uint8_t x_reg) {
    // TODO make toggle for below behavior
    // classic behavior modifies I
    // modern behavior doesn't
    uint16_t addr = I;
    uint16_t* addr_ptr = &addr;
    if (!config.quirks.memory_leave_I_unchanged) {
        addr_ptr = &I;
    }
    for (uint8_t reg = 0; reg <= x_reg; reg++) {
        memory[*addr_ptr] = registers[reg];
        *addr_ptr += 1;
    }
    if (config.quirks.memory_increment_by_X) {
        *addr_ptr -= 1;
    }
}

//(FX65) read contents of memory at I into V0 to VX (classic)
void CPU::read_mem_reg(uint8_t x_reg) {
    // TODO make toggle for below behavior
    // classic behavior modifies I
    // modern behavior doesn't
    uint16_t addr = I;
    uint16_t* addr_ptr = &addr;
    if (!config.quirks.memory_leave_I_unchanged) {
        addr_ptr = &I;
    }
    for (uint8_t reg = 0; reg <= x_reg; reg++) {
        registers[reg] = memory[*addr_ptr];
        *addr_ptr += 1;
    }
    if (config.quirks.memory_increment_by_X) {
        *addr_ptr -= 1;
    }
}

//[SCHIP-8-1.1]

//(00CN) scroll screen down by N pixels
void CPU::scroll_down_n(uint8_t val) {
    std::lock_guard<std::mutex> lock(screen_mtx);
    // start from bottom and replace with n heigher if in bounds else set to 0
    if (lores && !config.quirks.half_scroll_lores) {
        val *= 2;
    }
    for (int row = HEIGHT - 1; row >= 0; row--) {
        for (int col = WIDTH - 1; col >= 0; col--) {
            int index = (row * WIDTH) + col;
            screen[index] &= ~(bit_plane);                       // zeroes out bits that will be changed
            if ((row - val) >= 0) {
                int replace_index = ((row - val) * WIDTH) + col;
                screen[index] |= bit_plane & screen[replace_index];  // places the scrolled bits in correct bit spot
            }
        }
    }
    screen_update = true;
}

//(00FB) scroll screen right by four pixels  (SCHIP Quirk: lores scrolls half)
void CPU::scroll_right_four() {
    std::lock_guard<std::mutex> lock(screen_mtx);
    uint8_t val = 4;
    if (lores && !config.quirks.half_scroll_lores) {
        val *= 2;
    }
    // traverse right to left top to down
    for (int row = 0; row < HEIGHT; row++) {
        for (int col = WIDTH - 1; col >= 0; col--) {
            int index = (row * WIDTH) + col;
            screen[index] &= ~(bit_plane);                       // zeroes out bits that will be changed
            if ((col - val) >= 0) {
                int replace_index = (row * WIDTH) + (col - val);
                screen[index] |= bit_plane & screen[replace_index];  // places the scrolled bits in correct bit spot
            } 
        }
    }
    screen_update = true;
}

//(00FC) scroll screen left by four pixels (SCHIP Quirk: lores scrolls half)
void CPU::scroll_Left_four() {
    std::lock_guard<std::mutex> lock(screen_mtx);
    uint8_t val = 4;
    if (lores && !config.quirks.half_scroll_lores) {
        val *= 2;
    }
    // traverse left to right top to down
    for (int row = 0; row < HEIGHT; row++) {
        for (int col = 0; col < WIDTH; col++) {
            int index = (row * WIDTH) + col;
            screen[index] &= ~(bit_plane);                       // zeroes out bits that will be changed
            if ((col + val) < WIDTH) {
                int replace_index = (row * WIDTH) + (col + val);
                screen[index] |= bit_plane & screen[replace_index];  // places the scrolled bits in correct bit spot
            }
       }
    }
    screen_update = true;
}

// TODO make this function return to start screen not kill render loop
//(00FD) exit interpreter
void CPU::exit() {
    // terminate();
    return;
}

//(00FE) switch to lores (64x32) mode
void CPU::switch_lores() {
    // SCHIP Quirk: original didnt clear screen
    if (config.quirks.clean_screen) {
        std::lock_guard<std::mutex> lock(screen_mtx);
        std::memset(screen, 0, WIDTH * HEIGHT * sizeof(uint8_t));
        screen_update = true;
    }
    lores = true;
}

//(00FF) switch to hires (128x64) mode
void CPU::switch_hires() {
    // SCHIP Quirk: original didnt clear screen
    if (config.quirks.clean_screen) {
        std::lock_guard<std::mutex> lock(screen_mtx);
        std::memset(screen, 0, WIDTH * HEIGHT * sizeof(uint8_t));
        screen_update = true;
    }
    lores = false;
}

//(BXNN) jump to XNN + V[X] (Note: this replaces BNNN which is used for classic
// and XO chip)
void CPU::jump_plus_reg(uint16_t addr, uint8_t x_reg) {
    uint8_t val = registers[x_reg];
    PC = addr + val;
}

//(FX30) set I to big font (10 line) for digit in lowest nibble of V[X]
void CPU::set_index_font_big(uint8_t x_reg) {
    uint8_t ch = registers[x_reg] & 0xF;
    I = (0x0A0 + (10 * ch));
}

//(FX75) write V[0] to V[X] in flags storage
void CPU::write_flags_storage(uint8_t x_reg) {
    for (uint8_t i = 0; i <= x_reg; i++) {
        flags[i] = registers[i];
    }
}

//(FX85) read flags[0] to flags[X] into registers
void CPU::read_flags_storage(uint8_t x_reg) {
    for (uint8_t i = 0; i <= x_reg; i++) {
        registers[i] = flags[i];
    }
}

//[XO-CHIP]

// 00DN scroll screen up by N pixels
void CPU::scroll_up_n(uint8_t val) {
    std::lock_guard<std::mutex> lock(screen_mtx);
    // scroll only selected bit planes
    //  start from bottom and replace with n lower if in bounds else set to 0
    if (lores) {
        val *= 2;
    }
    for (int row = 0; row < HEIGHT; row++) {
        for (int col = 0; col < WIDTH; col++) {
            int index = (row * WIDTH) + col;
            screen[index] &= ~(bit_plane);                       // zeroes out bits that will be changed
            if ((row + val) < HEIGHT) {
                int replace_index = ((row + val) * WIDTH) + col;
                screen[index] |= bit_plane & screen[replace_index];  // places the scrolled bits in correct bit spot
            }
        }
    }
    screen_update = true;
}

// TODO recheck implementation if something breaking
// 5XY2 write memory starting from register X to register y
void CPU::write_reg_mem_range(uint8_t x_reg, uint8_t y_reg) {
    uint16_t addr = I;
    int8_t inc = x_reg <= y_reg ? 1 : -1;

    for (uint8_t reg = x_reg; reg != y_reg; reg += inc) {
        memory[addr] = registers[reg];
        addr += 1;
    }
    memory[addr] = registers[y_reg];
}

// TODO recheck implementation if something breaking
// 5XY2 read memory starting at I into register X to register y
void CPU::read_reg_mem_range(uint8_t x_reg, uint8_t y_reg) {
    uint16_t addr = I;
    int8_t inc = x_reg <= y_reg ? 1 : -1;

    for (uint8_t reg = x_reg; reg != y_reg; reg += inc) {
        registers[reg] = memory[addr];
        addr += 1;
    }
    registers[y_reg] = memory[addr];
}

// TODO test this if things are breaking
// FOOO NNNN read the next two bytes into I
void CPU::set_index_long() {
    I = fetch();
}

// FX01 selects the bit plane VX to use for drawing and scrolling
void CPU::select_plane(uint8_t val) {
    bit_plane = val;
}

// F002 load 16 byte audio pattern pointed by I into audio pattern buffer
void CPU::set_waveform() {
    // TODO implement this
}

// FX3A set playback rate to 4000*2^((vX-64)/48)Hz
void CPU::set_pitch(uint8_t x_reg) {
    // TODO implement this
}