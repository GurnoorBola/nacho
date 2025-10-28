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
    unsigned char memory[MAX_MEM] = {};
    unsigned char screen[WIDTH*HEIGHT] = {};
    unsigned short PC;
    unsigned short I;
    unsigned short stack[MAX_STACK] = {};
    unsigned char SP = -1;
    int last = 0;
    int delay;
    int sound;
    unsigned char registers[16] = {};

    unsigned char pressed;
    unsigned char old;

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void process_input();

    //stack operations
    void push(unsigned short x);
    unsigned short pop();
    unsigned short peek();
   
    //CHIP-8 Functionality 
    unsigned short fetch();
    void decode(unsigned short instruction);


    //opcodes
    void clear(); //00E0 Clear Screen
    void return_subroutine(); //00EE Return from subroutine
    void jump(unsigned short addr); //1NNN Jump
    void start_subroutine(unsigned short addr); //2NNN Start subroutine at NNN
    void skip_equals(unsigned char x_reg, unsigned char val); //3XNN skip if VX == NN
    void skip_not_equals(unsigned char x_reg, unsigned char val); //4XNN skip if VX != NN
    void skip_reg_equals(unsigned char x_reg, unsigned char y_reg); //5XY0 skip if VX == VY
    void set(unsigned char x_reg, unsigned char val); //6XNN set register VX to NN
    void add(unsigned char x_reg, unsigned char val); //7XNN add NN to register VX
    void set_reg_equals(unsigned char x_reg, unsigned char y_reg); //8XY0 set VX to value of VY 
    void set_reg_or(unsigned char x_reg, unsigned char y_reg); //8XY1 set VX to or of value of VX and VY 
    void set_reg_and(unsigned char x_reg, unsigned char y_reg); //8XY2 set VX to and of value of VX and VY 
    void set_reg_xor(unsigned char x_reg, unsigned char y_reg); //8XY3 set VX to xor of value of VX and VY 
    void set_reg_sum(unsigned char x_reg, unsigned char y_reg); //8XY4 set VX to sum of value of VX and VY and set VF to 1 if overflow, 0 else
    void set_reg_sub_Y(unsigned char x_reg, unsigned char y_reg); //8XY5 set VX to diff of value of VX and VY and set VF to 0 if underflow, 1 else
    void set_reg_shift_right(unsigned char x_reg, unsigned char y_reg); //8XY6 set VX to value of VY, shift VX by a bit to right and set VF to bit shifted out
    void set_reg_sub_X(unsigned char x_reg, unsigned char y_reg); //8XY7 set VX to value of VY - VX and set VF to 0 if underflow, 1 else 
    void set_reg_shift_left(unsigned char x_reg, unsigned char y_reg); //8XYE set VX to value of VY, shift VX by a bit to left and set VF to bit shifted out
    void skip_reg_not_equals(unsigned char x_reg, unsigned char y_reg); //9XY0 skip if VX != VY
    void set_index(unsigned short addr); //ANNN set index to NNN
    void jump_plus(unsigned short addr); //BNNN jump to NNN + V0
    void set_reg_rand(unsigned char x_reg, unsigned char val); //CXNN set VX to random byte (bitwise AND) NN
    void display(unsigned char x_reg, unsigned char y_reg, unsigned char height); //DXYH draw character at x, y
    void skip_key_pressed(unsigned char x_reg); //EX9E skip if key represented by VX's lower nibble is pressed
    void skip_key_not_pressed(unsigned char x_reg); //EXA1 skip if key represented by VX's lower nibble is not pressed
    void set_reg_delay(unsigned char x_reg); //FX07 set VX to value of delay timer
    void set_reg_keypress(unsigned char x_reg); //FX0A wait for key press and release and set VX to that key
    void set_delay(unsigned char x_reg); //FX15 set delay timer to VX
    void set_sound(unsigned char x_reg); //FX18 set sound timer to VX
    void add_index(unsigned char x_reg); //FX1E add VX to I
    void set_index_font(unsigned char x_reg); //FX29 set I to memory location of character represented by lower nibble of VX
    void set_reg_BCD(unsigned char x_reg); //FX33 set memory at I, I+1, I+2 to be the hundredths, tens, and ones place of the decimal representation of VX
    void write_reg_mem(unsigned char x_reg); //FX55 write contents of V0 to VX to memory at I and I incremented by X+1
    void read_mem_reg(unsigned char x_reg); //FX65 read contents of memory at I into V0 to VX and I incremented by X+1

    //opengl window initialization
    GLFWwindow* window = NULL;
    Shader shader;
   
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;

    unsigned int texture;
    
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

public:
    Chip8();

    //each bit maps to keypress
    unsigned short keys = 0;

    //flag to stop emulation
    bool stop = false;
    
    int initDisplay();

    //IO functionality
    int loadProgram(std::string filename);

    void emulate_cycle();

    void terminate();
};