#include <display/display.h>

#include <cstring>

#include "cpu/cpu.h"
#include "miniaudio.h"

/*-----------------[Special Member Functions]-----------------*/

Display::Display(CPU& cpu) : core(cpu), gui(core) {
    init_display();
    gui.init_gui(window);
    init_audio();
}

/*-----------------[Window]-----------------*/

void Display::init_display() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WIDTH * SCALE, (HEIGHT + OFFSET) * SCALE, "CHIP-8", NULL, NULL);
    if (!window) {
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwSetWindowAspectRatio(window, WIDTH, (HEIGHT + OFFSET));
    glfwSetWindowSizeLimits(window, WIDTH * SCALE, (HEIGHT + OFFSET) * SCALE, GLFW_DONT_CARE, GLFW_DONT_CARE);

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, Display::key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glViewport(0, 0, WIDTH * SCALE, (HEIGHT + OFFSET) * SCALE);
    glfwSetFramebufferSizeCallback(window, Display::framebuffer_size_callback);

    // create shader object
    shader = Shader("shaders/shader.vs", "shaders/shader.fs");
    shader.use();

    float top = 1.0 - (2.0 * OFFSET) / (HEIGHT + OFFSET);

    float vertices[] = {
        // positions            // texture coords
        1.0f,  top,   0.0f, 1.0f, 1.0f,  // top right
        1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom left
        -1.0f, top,   0.0f, 0.0f, 1.0f   // top left
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, WIDTH, HEIGHT, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE,
                 core.get_screen().data());

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

    // set on and off color uniforms
    for (int i = 0; i < 16; i++) {
        shader.setVec3fv("color" + std::to_string(i), core.config.colors[i].data());
    }
}

void Display::render_loop() {
    while (!glfwWindowShouldClose(window)) {
        if (core.check_stop()) {
            break;
        }

        if (core.check_color()) {
            update_colors();
        }

        if (core.check_screen() == true) {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RED_INTEGER, GL_UNSIGNED_BYTE,
                            core.get_screen().data());
        }

        gui.update();

        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);

        gui.render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Display::update_colors() {
    shader.use();

    for (int i = 0; i < 16; i++) {
        shader.setVec3fv("color" + std::to_string(i), core.config.colors[i].data());
    }
}

void Display::terminate() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &EBO);

    ma_device_uninit(&device);

    // ImGui_ImplOpenGL3_Shutdown();
    // ImGui_ImplGlfw_Shutdown();
    // ImGui::DestroyContext();

    glfwTerminate();
}

void Display::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void Display::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (window == NULL) {
        std::cerr << "GLFW window not intialized" << std::endl;
        return;
    }
    Display* display = static_cast<Display*>(glfwGetWindowUserPointer(window));
    CPU& cpu = display->core;
    switch (action) {
        case GLFW_PRESS:
            switch (key) {
                case GLFW_KEY_ESCAPE:
                    cpu.terminate();
                    break;

                case GLFW_KEY_1:
                    cpu.press_key(0x1);
                    break;

                case GLFW_KEY_2:
                    cpu.press_key(0x2);
                    break;

                case GLFW_KEY_3:
                    cpu.press_key(0x3);
                    break;

                case GLFW_KEY_4:
                    cpu.press_key(0xC);
                    break;

                case GLFW_KEY_Q:
                    cpu.press_key(0x4);
                    break;

                case GLFW_KEY_W:
                    cpu.press_key(0x5);
                    break;

                case GLFW_KEY_E:
                    cpu.press_key(0x6);
                    break;

                case GLFW_KEY_R:
                    cpu.press_key(0xD);
                    break;

                case GLFW_KEY_A:
                    cpu.press_key(0x7);
                    break;

                case GLFW_KEY_S:
                    cpu.press_key(0x8);
                    break;

                case GLFW_KEY_D:
                    cpu.press_key(0x9);
                    break;

                case GLFW_KEY_F:
                    cpu.press_key(0xE);
                    break;

                case GLFW_KEY_Z:
                    cpu.press_key(0xA);
                    break;

                case GLFW_KEY_X:
                    cpu.press_key(0x0);
                    break;

                case GLFW_KEY_C:
                    cpu.press_key(0xB);
                    break;

                case GLFW_KEY_V:
                    cpu.press_key(0xF);
                    break;
            }
            break;

        case GLFW_RELEASE:
            switch (key) {
                case GLFW_KEY_1:
                    cpu.release_key(0x1);
                    break;

                case GLFW_KEY_2:
                    cpu.release_key(0x2);
                    break;

                case GLFW_KEY_3:
                    cpu.release_key(0x3);
                    break;

                case GLFW_KEY_4:
                    cpu.release_key(0xC);
                    break;

                case GLFW_KEY_Q:
                    cpu.release_key(0x4);
                    break;

                case GLFW_KEY_W:
                    cpu.release_key(0x5);
                    break;

                case GLFW_KEY_E:
                    cpu.release_key(0x6);
                    break;

                case GLFW_KEY_R:
                    cpu.release_key(0xD);
                    break;

                case GLFW_KEY_A:
                    cpu.release_key(0x7);
                    break;

                case GLFW_KEY_S:
                    cpu.release_key(0x8);
                    break;

                case GLFW_KEY_D:
                    cpu.release_key(0x9);
                    break;

                case GLFW_KEY_F:
                    cpu.release_key(0xE);
                    break;

                case GLFW_KEY_Z:
                    cpu.release_key(0xA);
                    break;

                case GLFW_KEY_X:
                    cpu.release_key(0x0);
                    break;

                case GLFW_KEY_C:
                    cpu.release_key(0xB);
                    break;

                case GLFW_KEY_V:
                    cpu.release_key(0xF);
                    break;
            }
            break;
    }
}

/*-----------------[Audio]-----------------*/

// Writes samples to ringbuffer
void Display::write_samples_callback() {
    // write to ring buffer
    ma_pcm_rb* pRB = &rb;
    ma_uint32 numSamples = SAMPLE_SIZE;

    void* pWriteBuffer;
    ma_pcm_rb_acquire_write(pRB, &numSamples, &pWriteBuffer);

    std::array<uint8_t, SAMPLE_SIZE> samples = core.gen_frame_samples();

    memcpy(pWriteBuffer, samples.data(), numSamples);

    ma_pcm_rb_commit_write(pRB, numSamples);

    ma_uint32 remaining = SAMPLE_SIZE - numSamples;

    if (remaining > 0) {
        ma_pcm_rb_acquire_write(pRB, &remaining, &pWriteBuffer);
        memcpy(pWriteBuffer, samples.data() + numSamples, remaining);
        ma_pcm_rb_commit_write(pRB, remaining);
    }
}

void Display::init_audio() {
    // set audio callback to write samples
    core.set_audio_callback([this]() { this->write_samples_callback(); });

    ma_device_config deviceConfig;

    ma_result result = ma_pcm_rb_init(DEVICE_FORMAT, DEVICE_CHANNELS, SAMPLE_SIZE * 8, NULL, NULL, &rb);
    if (result != MA_SUCCESS) {
        throw std::runtime_error("Failed to initialize ring buffer");
    }

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = DEVICE_FORMAT;
    deviceConfig.playback.channels = DEVICE_CHANNELS;
    deviceConfig.sampleRate = DEVICE_SAMPLE_RATE;
    deviceConfig.dataCallback = data_callback;
    deviceConfig.pUserData = &rb;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        throw std::runtime_error("Failed to initialize audio device");
    }
    ma_device_start(&device);
}

void Display::data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    ma_pcm_rb* pRB;

    assert(pDevice->playback.channels == DEVICE_CHANNELS);

    pRB = (ma_pcm_rb*)pDevice->pUserData;
    assert(pRB != NULL);

    ma_uint32 framesRead = frameCount;

    void* pInputBuffer;
    if (ma_pcm_rb_acquire_read(pRB, &framesRead, &pInputBuffer) != MA_SUCCESS) {
        ma_silence_pcm_frames(pOutput, frameCount, DEVICE_FORMAT, DEVICE_CHANNELS);
        return;
    }
    memcpy(pOutput, pInputBuffer, framesRead);
    ma_pcm_rb_commit_read(pRB, framesRead);

    ma_uint32 remaining = frameCount - framesRead;
    if (remaining > 0) {
        if (ma_pcm_rb_acquire_read(pRB, &remaining, &pInputBuffer) != MA_SUCCESS) {
            ma_silence_pcm_frames((void*)((char*)pOutput + framesRead), frameCount - framesRead, DEVICE_FORMAT, DEVICE_CHANNELS);
            ma_pcm_rb_commit_read(pRB, frameCount - framesRead);
            return;
        }
        memcpy((void*)((char*)pOutput + framesRead), pInputBuffer, remaining);
        ma_pcm_rb_commit_read(pRB, remaining);
    }
    (void)pInput;
}
