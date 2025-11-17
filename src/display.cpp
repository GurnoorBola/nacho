#include <display/display.h>

/*-----------------[Special Member Functions]-----------------*/

Display::Display(CPU& cpu) : core(cpu), gui(core) {
    init_display();
    gui.init_gui(window);
    init_audio();
}

/*-----------------[Window]-----------------*/

void Display::init_display() {
    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, 0);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WIDTH * 10, HEIGHT * 10, "CHIP-8", NULL, NULL);
    if (!window) {
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, Display::key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glViewport(0, 0, WIDTH * 10, HEIGHT * 10);
    glfwSetFramebufferSizeCallback(window, Display::framebuffer_size_callback);

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, WIDTH, HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, core.get_screen());

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

    //set on and off color uniforms
    shader.setVec3fv("onColor", core.config.onColor.data());
    shader.setVec3fv("offColor", core.config.offColor.data());
}

void Display::render_loop() {
    while (!glfwWindowShouldClose(window)) {
        if (core.check_stop()) {
            break;
        }

        uint8_t* screen = core.check_screen();
        if (screen != NULL) {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RED, GL_UNSIGNED_BYTE, screen);
        }

        switch (core.check_should_beep()) {
            case START_BEEP:
                if (ma_device_start(&device) != MA_SUCCESS) {
                    std::cerr << "Failed to start playback device." << std::endl;
                    ma_device_uninit(&device);
                }
                break;

            case STOP_BEEP:
                if (ma_device_stop(&device) != MA_SUCCESS) {
                    std::cerr << "Failed to stop playback device." << std::endl;
                    ma_device_uninit(&device);
                }
                break;
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

void Display::update_config(){
    shader.use();
    //set on and off color uniforms
    shader.setVec3fv("onColor", core.config.onColor.data());
    shader.setVec3fv("offColor", core.config.offColor.data());
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
    glViewport(0, 0, WIDTH * 10, HEIGHT * 10);
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

void Display::init_audio() {
    ma_device_config deviceConfig;
    ma_waveform_config squareWaveConfig;

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = DEVICE_FORMAT;
    deviceConfig.playback.channels = DEVICE_CHANNELS;
    deviceConfig.sampleRate = DEVICE_SAMPLE_RATE;
    deviceConfig.dataCallback = data_callback;
    deviceConfig.pUserData = &squareWave;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        throw std::runtime_error("Failed to initialize audio device");
    }

    squareWaveConfig = ma_waveform_config_init(device.playback.format, device.playback.channels, device.sampleRate,
                                               ma_waveform_type_square, 0.2, 440);
    ma_waveform_init(&squareWaveConfig, &squareWave);
}

void Display::data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    ma_waveform* pSquareWave;

    assert(pDevice->playback.channels == DEVICE_CHANNELS);

    pSquareWave = (ma_waveform*)pDevice->pUserData;
    assert(pSquareWave != NULL);

    ma_waveform_read_pcm_frames(pSquareWave, pOutput, frameCount, NULL);

    (void)pInput;
}
