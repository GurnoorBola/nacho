#include <cpu/cpu.h>
#include <database/database.h>
#include <openssl/sha.h>

#include <fstream>
#include <unordered_map>

std::unordered_map<std::string, int> supported = {{"originalChip8", CHIP8},
                                                  {"modernChip8", SCHIP_MODERN},
                                                  {"superchip", SCHIP1_1},
                                                  {"xochip", XO_CHIP}};

Database::Database(std::string data_dir) {
    // open files and parse into json objects
    std::ifstream platform_f(data_dir + "/platforms.json");
    std::ifstream programs_f(data_dir + "/programs.json");
    std::ifstream quirks_f(data_dir + "/quirks.json");
    std::ifstream sha1_hashes_f(data_dir + "/sha1-hashes.json");

    platforms = json::parse(platform_f);
    programs = json::parse(programs_f);
    quirk_list = json::parse(quirks_f);
    sha1_hashes = json::parse(sha1_hashes_f);
};

void Database::set_platform_quirks(CPU::Config& config, int platform) {
    // set platform defaults
    json& platform_data = platforms[platform];
    json& platform_quirks = platform_data["quirks"];

    config.system = platform;
    config.speed = platform_data["defaultTickrate"];
    config.quirks.shift = platform_quirks["shift"];
    config.quirks.memory_increment_by_X = platform_quirks["memoryIncrementByX"];
    config.quirks.memory_leave_I_unchanged = platform_quirks["memoryLeaveIUnchanged"];
    config.quirks.wrap = platform_quirks["wrap"];
    config.quirks.jump = platform_quirks["jump"];
    config.quirks.vblank = platform_quirks["vblank"];
    config.quirks.logic = platform_quirks["logic"];
    config.quirks.draw_zero = platform_quirks["drawZero"];
    config.quirks.half_scroll_lores = platform_quirks["halfScrollLores"];
    config.quirks.clean_screen = platform_quirks["cleanScreen"];
    config.quirks.set_collisions = platform_quirks["setCollisions"];
    config.quirks.lores_8x16 = platform_quirks["lores8x16"];
}

void Database::set_game_quirks(CPU::Config& config, json& game_rom) {
    int tick_rate = game_rom.value("tickrate", -1);
    if (tick_rate != -1) config.speed = tick_rate;
    int start_address = game_rom.value("startAddress", -1);
    if (start_address != -1) config.start_address = start_address;
    // TODO add more advanced things from json
    json colors = game_rom.value("colors", json(nullptr));
    if (colors != nullptr) {
        std::vector<std::string> pixels = colors.value("pixels", std::vector<std::string>());

        int length = pixels.size();
        for (int i=0; i < length; i++) {
           config.colors[i] = hex_to_rgb(pixels[i]);
        }
    }
}

std::array<float, 3> Database::hex_to_rgb(std::string hex) {
    std::array<float, 3> rgb;
    rgb[0] = std::stoi(hex.substr(1, 2), nullptr, 16) / 255.0f;
    rgb[1] = std::stoi(hex.substr(3, 2), nullptr, 16) / 255.0f;
    rgb[2] = std::stoi(hex.substr(5, 2), nullptr, 16) / 255.0f;
    return rgb;
}

// generates a configuration and sets internal variables to store quick info related to the game
// will choose best system to emulate automatically.
CPU::Config Database::gen_config(std::string hash) {
    std::cout << hash << std::endl;
    CPU::Config config;

    game_index = sha1_hashes.value(hash, -1);
    if (game_index == -1) {
        std::cout << "Game not found in database. Please select a system..." << std::endl;
        return config;
    }
    std::cout << "Game found at index " << game_index << std::endl;

    json& game_meta = programs[game_index];
    json& game_rom = game_meta["roms"][hash];
    std::vector<std::string> platform_list = game_rom["platforms"];
    std::string platform_name = "";
    for (std::string p : platform_list) {
        if (supported.find(p) != supported.end()) {
            platform_name = p;
        }
    }

    if (platform_name.empty()) {
        std::cout << "Game: " << game_meta["title"] << " is not spported yet :(" << std::endl;
        return config;
    }

    std::cout << "Platform " << platform_name << std::endl;

    platform = supported[platform_name];

    // set platform quirks
    set_platform_quirks(config, platform);

    // set game specific quirks
    set_game_quirks(config, game_rom);

    return config;
}


CPU::Config Database::gen_platform_config(int platform){
    CPU::Config config;
    set_platform_quirks(config, platform);
    return config;
}