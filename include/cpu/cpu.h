#pragma once

#include <json.hpp>
#include <mutex>
#include <atomic>

#define MAX_MEM 65535 
#define MAX_STACK 16
#define WIDTH 128
#define HEIGHT 64
#define SCREEN_SIZE (WIDTH * HEIGHT)

//Sample_size = audio rate / frame rate
#define SAMPLE_SIZE 48000 / 60 
#define DEVICE_SAMPLE_RATE 48000

#define NUM_SYSTEMS 4
#define CHIP8 0
#define SCHIP_MODERN 2
#define SCHIP1_1 6
#define XO_CHIP 8

#define MAX_AMPLITUDE 192

#define NO_PRESS 0
#define NO_RELEASE 255 

class CPU {
   public:
    CPU();

    // each bit maps to keypress
    uint16_t keys = 0;

    struct Quirks {
        bool shift = false;
        bool memory_increment_by_X = false;
        bool memory_leave_I_unchanged = false;
        bool wrap = false;
        bool jump = false;
        bool vblank = false;
        bool logic = false;
        bool draw_zero = false;
        bool half_scroll_lores = false;
        bool clean_screen = true;
        bool set_collisions = false;
        bool lores_8x16 = false;
    };

    struct Config {
        int system = SCHIP_MODERN;
        int speed = 12;
        
        //default color pallete
        std::array<std::array<float, 3>, 16> colors = {{
            {0/255.0, 0/255.0, 0/255.0},
            {157/255.0, 157/255.0, 157/255.0},
            {255/255.0, 255/255.0, 255/255.0},
            {190/255.0, 38/255.0, 51/255.0},
            {224/255.0, 111/255.0, 139/255.0},
            {73/255.0, 60/255.0, 43/255.0},
            {164/255.0, 100/255.0, 34/255.0},
            {235/255.0, 137/255.0, 49/255.0},
            {247/255.0, 226/255.0, 107/255.0},
            {47/255.0, 72/255.0, 78/255.0},
            {68/255.0, 137/255.0, 26/255.0},
            {163/255.0, 206/255.0, 39/255.0},
            {27/255.0, 38/255.0, 50/255.0},
            {0/255.0, 87/255.0, 132/255.0},
            {49/255.0, 162/255.0, 242/255.0},
            {178/255.0, 220/255.0, 239/255.0}
        }};

        uint16_t start_address = 0x200;

        Quirks quirks;
    };

    Config config; // *

    

    // IO functionality

    int loadProgram(std::string filepath);
    std::string hash_bin(int fileSize);

    // Main CHIP8 Functionality
    void emulate_cycle();
    void emulate_loop();
    void pause();
    void resume();
    void step();
    void terminate();

    // Access functions

    void press_key(uint8_t key);
    void release_key(uint8_t key);

    // check for update and return screen if updated else return NULL
    std::array<uint8_t, SCREEN_SIZE> get_screen();
    bool check_screen();
    std::array<uint8_t, SAMPLE_SIZE> gen_frame_samples();
    void set_audio_callback(std::function<void(void)> callback);

    bool check_stop();
    bool check_color();

    void set_config(Config config);

    void reset();

    nlohmann::json gen_save();
    int load_save(std::ifstream& file);

    void dump_reg();

   private:
    std::array<uint8_t, MAX_MEM> memory {}; // *
    std::array<uint8_t, SCREEN_SIZE> screen {}; // *
    uint16_t PC; // *
    uint16_t I; // *
    std::array<uint16_t, MAX_STACK> stack {}; // *
    int SP = -1; // *
    int delay = 0; // *
    int sound = 0; // *
    std::array<uint8_t, 16> registers {}; // *

    std::array<uint8_t, 16> flags {}; // *

    std::array<uint8_t, 128> audio_pattern {}; // *
    float playback_rate = 4000; // *
    float phase = 0; // *
    std::function<void(void)> audio_callback;

    bool lores = true; // *

    uint8_t bit_plane = 0b01; // *

    //keys currently pressed
    uint16_t pressed = NO_PRESS;
    //last released
    uint8_t released = NO_RELEASE;

    bool waiting = false;

    Quirks quirks;

    std::mutex screen_mtx;
    std::mutex key_mtx;

    // flags

    // flag to stop emulation
    std::atomic<bool> stop = false;
    // bool to notify if we drew
    std::atomic<bool> draw = false;
    // bool if screen updated
    std::atomic<bool> screen_update = false;
    //if colors updated in config
    std::atomic<bool> color_update = false;

    std::atomic<bool> paused = false;

    // stack operations
    void push(uint16_t x);
    uint16_t pop();
    uint16_t peek();

    // CHIP-8 Functionality
    uint16_t fetch();
    void decode(uint16_t instruction);
    void decrementTimers();

    //[chip8] opcodes
    void clear();                                        // 00E0 Clear Screen
    void return_subroutine();                            // 00EE Return from subroutine
    void jump(uint16_t addr);                            // 1NNN Jump
    void start_subroutine(uint16_t addr);                // 2NNN Start subroutine at NNN
    void skip_equals(uint8_t x_reg, uint8_t val);        // 3XNN skip if VX == NN
    void skip_not_equals(uint8_t x_reg, uint8_t val);    // 4XNN skip if VX != NN
    void skip_reg_equals(uint8_t x_reg, uint8_t y_reg);  // 5XY0 skip if VX == VY
    void set(uint8_t x_reg, uint8_t val);                // 6XNN set register VX to NN
    void add(uint8_t x_reg, uint8_t val);                // 7XNN add NN to register VX
    void set_reg_equals(uint8_t x_reg,
                        uint8_t y_reg);  // 8XY0 set VX to value of VY
    void set_reg_or(uint8_t x_reg,
                    uint8_t y_reg);  // 8XY1 set VX to or of value of VX and VY
    void set_reg_and(uint8_t x_reg,
                     uint8_t y_reg);  // 8XY2 set VX to and of value of VX and VY
    void set_reg_xor(uint8_t x_reg,
                     uint8_t y_reg);  // 8XY3 set VX to xor of value of VX and VY
    void set_reg_sum(uint8_t x_reg,
                     uint8_t y_reg);  // 8XY4 set VX to sum of value of VX and VY
                                      // and set VF to 1 if overflow, 0 else
    void set_reg_sub_Y(uint8_t x_reg,
                       uint8_t y_reg);  // 8XY5 set VX to diff of value of VX and
                                        // VY and set VF to 0 if underflow, 1 else
    void set_reg_shift_right(uint8_t x_reg,
                             uint8_t y_reg);  // 8XY6 set VX to value of VY, shift VX by a bit to right
                                              // and set VF to bit shifted out
    void set_reg_sub_X(uint8_t x_reg,
                       uint8_t y_reg);  // 8XY7 set VX to value of VY - VX and set
                                        // VF to 0 if underflow, 1 else
    void set_reg_shift_left(uint8_t x_reg,
                            uint8_t y_reg);  // 8XYE set VX to value of VY, shift VX by a bit to left
                                             // and set VF to bit shifted out
    void skip_reg_not_equals(uint8_t x_reg,
                             uint8_t y_reg);  // 9XY0 skip if VX != VY
    void set_index(uint16_t addr);            // ANNN set index to NNN
    void jump_plus(uint16_t addr);            // BNNN jump to NNN + V0
    void set_reg_rand(uint8_t x_reg,
                      uint8_t val);  // CXNN set VX to random byte (bitwise AND) NN
    void display(uint16_t mem_index, uint8_t plane, uint8_t x_reg, 
                uint8_t y_reg, uint8_t width, uint8_t height);  // if height is 0 we draw 16 x 16 otherise draw 8 x height 
    void skip_key_pressed(uint8_t x_reg);      // EX9E skip if key represented by VX's
                                               // lower nibble is pressed
    void skip_key_not_pressed(uint8_t x_reg);  // EXA1 skip if key represented by
                                               // VX's lower nibble is not pressed
    void set_reg_delay(uint8_t x_reg);         // FX07 set VX to value of delay timer
    void set_reg_keypress(uint8_t x_reg);      // FX0A wait for key press and release
                                               // and set VX to that key
    void set_delay(uint8_t x_reg);             // FX15 set delay timer to VX
    void set_sound(uint8_t x_reg);             // FX18 set sound timer to VX
    void add_index(uint8_t x_reg);             // FX1E add VX to I
    void set_index_font(uint8_t x_reg);        // FX29 set I to memory location of character represented
                                               // by lower nibble of VX
    void set_reg_BCD(uint8_t x_reg);           // FX33 set memory at I, I+1, I+2 to be the
                                               // hundredths, tens, and ones place of the
                                               // decimal representation of VX
    void write_reg_mem(uint8_t x_reg);         // FX55 write contents of V0 to VX to
                                               // memory at I and I incremented by X+1
    void read_mem_reg(uint8_t x_reg);          // FX65 read contents of memory at I into
                                               // V0 to VX and I incremented by X+1

    //[schip8] opcodes (1.1)
    void scroll_down_n(uint8_t val);  // 00CN scroll screen down by N pixels
                                      // (SCHIP Quirk: lores scrolls half)
                                      // (XOCHIP Quirk: only slected bit plane scrolls XO CHIP)
    void scroll_right_four();         // 00FB scroll screen right by four pixels
                                      // (SCHIP Quirk: lores scrolls half)
                                      // (XOCHIP Quirk: only slected bit plane scrolls XO CHIP)
    void scroll_Left_four();          // 00FC scroll screen left by four pixels
                                      // (SCHIP Quirk: lores scrolls half)
                                      // (XOCHIP Quirk: only slected bit plane scrolls XO CHIP)
    void exit();                      // 00FD exit interpreter
    void switch_lores();              // 00FE switch to lores (64x32) mode (SCHIP Quirk:
                                      // original didnt clear screen)(but I will MODERN)
    void switch_hires();              // 00FF switch to hires (128x64) mode (SCHIP Quirk:
                                      // original didnt clear screen)(but I will MODERN)
    void jump_plus_reg(uint16_t addr,
                       uint8_t x_reg);  // BXNN jump to XNN + V[X] (Note: this replaces BNNN
                                        // which is used for classic and XO chip)
    void set_index_font_big(uint8_t x_reg);   // FX30 set I to big font (10 line) for digit in
                                              // lowest nibble of V[X]
    void write_flags_storage(uint8_t x_reg);  // FX75 write V[0] to V[X] in flags storage
    void read_flags_storage(uint8_t x_reg);   // FX85 read flags[0] to flags[X] into registers

    //[xo-chip] opcodes

    void scroll_up_n(uint8_t val);                           // 00DN scroll screen up by N pixels
                                                             // (XOCHIP Quirk: only slected bit plane scrolls XO CHIP)
    void write_reg_mem_range(uint8_t x_reg, uint8_t y_reg);  // 5XY2 write memory starting from register X to register y
                                                             // into memory at I X is not necessarily less than Y
    void read_reg_mem_range(uint8_t x_reg, uint8_t y_reg);  // 5XY2 read memory starting at I into register X to
                                                            // register y X is not necessarily less than Y
    void set_index_long();  // FOOO NNNN read the next two bytes into I
                            // increment to start of next instruction skipping NNNN
    void select_plane(uint8_t val);  // FX01 select the bit plane to use for drawing and scrolling
    void set_waveform();               // F002 load 16 byte audio pattern pointed by I into audio pattern buffer
    void set_pitch(uint8_t x_reg);                  // FX3A set playback rate to 4000*2^((vX-64)/48)Hz
};
