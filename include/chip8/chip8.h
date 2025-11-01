#pragma once
#include <iostream>
#include <glad.h>
#include <glfw3.h>
#include <shaders/shader.h>
#include <cmath>

#define MAX_MEM 4096
#define MAX_STACK 16
#define WIDTH 64
#define HEIGHT 32

class Chip8 {

private:
    uint8_t memory[MAX_MEM] = {};
    uint8_t screen[WIDTH*HEIGHT] = {};
    uint16_t PC;
    uint16_t I;
    uint16_t stack[MAX_STACK] = {};
    uint8_t SP = -1;
    uint8_t delay = 0;
    uint8_t sound = 0;
    uint8_t registers[16] = {};

    uint8_t flags[16] = {};
    bool lores = false;

    uint8_t pressed = 0xFF;
    bool waiting = false;

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

    //stack operations
    void push(uint16_t x);
    uint16_t pop();
    uint16_t peek();
   
    //CHIP-8 Functionality 
    uint16_t fetch();
    void decode(uint16_t instruction);


    //chip8 opcodes
    void clear(); //00E0 Clear Screen
    void return_subroutine(); //00EE Return from subroutine
    void jump(uint16_t addr); //1NNN Jump
    void start_subroutine(uint16_t addr); //2NNN Start subroutine at NNN
    void skip_equals(uint8_t x_reg, uint8_t val); //3XNN skip if VX == NN
    void skip_not_equals(uint8_t x_reg, uint8_t val); //4XNN skip if VX != NN
    void skip_reg_equals(uint8_t x_reg, uint8_t y_reg); //5XY0 skip if VX == VY
    void set(uint8_t x_reg, uint8_t val); //6XNN set register VX to NN
    void add(uint8_t x_reg, uint8_t val); //7XNN add NN to register VX
    void set_reg_equals(uint8_t x_reg, uint8_t y_reg); //8XY0 set VX to value of VY 
    void set_reg_or(uint8_t x_reg, uint8_t y_reg); //8XY1 set VX to or of value of VX and VY 
    void set_reg_and(uint8_t x_reg, uint8_t y_reg); //8XY2 set VX to and of value of VX and VY 
    void set_reg_xor(uint8_t x_reg, uint8_t y_reg); //8XY3 set VX to xor of value of VX and VY 
    void set_reg_sum(uint8_t x_reg, uint8_t y_reg); //8XY4 set VX to sum of value of VX and VY and set VF to 1 if overflow, 0 else
    void set_reg_sub_Y(uint8_t x_reg, uint8_t y_reg); //8XY5 set VX to diff of value of VX and VY and set VF to 0 if underflow, 1 else
    void set_reg_shift_right(uint8_t x_reg, uint8_t y_reg); //8XY6 set VX to value of VY, shift VX by a bit to right and set VF to bit shifted out
    void set_reg_sub_X(uint8_t x_reg, uint8_t y_reg); //8XY7 set VX to value of VY - VX and set VF to 0 if underflow, 1 else 
    void set_reg_shift_left(uint8_t x_reg, uint8_t y_reg); //8XYE set VX to value of VY, shift VX by a bit to left and set VF to bit shifted out
    void skip_reg_not_equals(uint8_t x_reg, uint8_t y_reg); //9XY0 skip if VX != VY
    void set_index(uint16_t addr); //ANNN set index to NNN
    void jump_plus(uint16_t addr); //BNNN jump to NNN + V0
    void set_reg_rand(uint8_t x_reg, uint8_t val); //CXNN set VX to random byte (bitwise AND) NN
    void display(uint8_t x_reg, uint8_t y_reg, uint8_t height); //DXYH draw character at x, y
    void skip_key_pressed(uint8_t x_reg); //EX9E skip if key represented by VX's lower nibble is pressed
    void skip_key_not_pressed(uint8_t x_reg); //EXA1 skip if key represented by VX's lower nibble is not pressed
    void set_reg_delay(uint8_t x_reg); //FX07 set VX to value of delay timer
    void set_reg_keypress(uint8_t x_reg); //FX0A wait for key press and release and set VX to that key
    void set_delay(uint8_t x_reg); //FX15 set delay timer to VX
    void set_sound(uint8_t x_reg); //FX18 set sound timer to VX
    void add_index(uint8_t x_reg); //FX1E add VX to I
    void set_index_font(uint8_t x_reg); //FX29 set I to memory location of character represented by lower nibble of VX
    void set_reg_BCD(uint8_t x_reg); //FX33 set memory at I, I+1, I+2 to be the hundredths, tens, and ones place of the decimal representation of VX
    void write_reg_mem(uint8_t x_reg); //FX55 write contents of V0 to VX to memory at I and I incremented by X+1
    void read_mem_reg(uint8_t x_reg); //FX65 read contents of memory at I into V0 to VX and I incremented by X+1

    //schip8 opcodes (1.1)
    void scroll_down_n(uint8_t val); //00CN scroll screen down by N pixels (SCHIP Quirk: lores scrolls half)
    void scroll_right_four(); //00FB scroll screen right by four pixels  (SCHIP Quirk: lores scrolls half)
    void scroll_Left_four(); //00FC scroll screen left by four pixels (SCHIP Quirk: lores scrolls half)
    void exit(); //00FD exit interpreter
    void switch_lores(); //00FE switch to lores (64x32) mode (SCHIP Quirk: original didnt clear screen)(but I will MODERN)
    void switch_hires(); //00FE switch to hires (128x64) mode (SCHIP Quirk: original didnt clear screen)(but I will MODERN)
    void jump_plus_reg(uint16_t addr, uint8_t x_reg); //BXNN jump to XNN + V[X] (Note: this replaces BNNN which is used for classic and XO chip)
    void draw_sprite(uint8_t x_reg, uint8_t y_reg); //DXY0 draw (16x16) sprite at V[X], V[Y] starting from I (see site for quirks)
    void set_index_font_big(); //FX30 set I to big font (10 line) for digit in lowest nibble of V[X]
    void write_flags_storage(uint8_t x_reg); //FX75 write V[0] to V[X] in flags storage
    void read_flags_storage(uint8_t x_reg); //FX75 write V[0] to V[X] in flags storage

    //opengl window initialization
    GLFWwindow* window = NULL;
    Shader shader;
   
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;

    unsigned int texture;
    
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

public:
    Chip8(int mode, int speed);

    //each bit maps to keypress
    uint16_t keys = 0;

    int mode;
    int speed;

    //flag to stop emulation
    bool stop = false;
    //bool to notify if we drew
    bool draw = false;
    
    int initDisplay();

    //IO functionality
    int loadProgram(std::string filename);

    void emulate_cycle();

    void decrementTimers();

    void terminate();
};