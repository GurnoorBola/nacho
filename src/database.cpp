#include <fstream>
#include <unordered_map>
#include <cpu/cpu.h>
#include <openssl/sha.h>
#include <database/database.h>

std::unordered_map<std::string, int> supported = {{"originalChip8",  CHIP8}, {"modernChip8", SCHIP_MODERN}, {"superchip", SCHIP1_1}};

Database::Database(std::string data_dir) {
    //open files and parse into json objects
    std::ifstream platform_f(data_dir + "/platforms.json");
    std::ifstream programs_f(data_dir + "/programs.json");
    std::ifstream quirks_f(data_dir + "/quirks.json");
    std::ifstream sha1_hashes_f(data_dir + "/sha1-hashes.json");

    platforms = json::parse(platform_f);
    programs = json::parse(programs_f);
    quirk_list = json::parse(quirks_f);
    sha1_hashes = json::parse(sha1_hashes_f);
};

//reads in bytes and generates the hash of the bytes
std::string hash_bin(std::string filename){
    std::ifstream program("games/" + filename, std::ios::binary);
    if (!program.is_open()) {
        std::cerr << "Program failed to open" << std::endl;
        return ""; 
    }
    program.seekg(0, std::ios::end);
    std::streampos fileSize = program.tellg();
    program.seekg(0, std::ios::beg);

    if (fileSize > MAX_PROG_SIZE) {
        std::cerr << "File size exceeds max program size" << std::endl;
        return ""; 
    }
    uint8_t bytes[MAX_PROG_SIZE];
    program.read(reinterpret_cast<char*>(bytes), fileSize);

    // compute hash
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(bytes, fileSize, hash);

    std::ostringstream oss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return oss.str();
}

void Database::set_platform_quirks(CPU::Config& config, int platform){
    //set platform defaults
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

void Database::set_game_quirks(CPU::Config& config, json& game_rom){
    int tick_rate = game_rom.value("tickrate", -1);
    if (tick_rate != -1) config.speed = tick_rate;
    int start_address = game_rom.value("startAddress", -1);
    if (start_address != -1) config.start_address = start_address;
    //TODO add more advanced things from json
    json colors = game_rom.value("colors", json(nullptr));
    if (colors != nullptr) {
        std::vector<std::string> pixels = colors.value("pixels", std::vector<std::string>()); 
        if (!pixels.empty()) {
            config.offColor = hex_to_rgb(pixels[0]);
            config.onColor = hex_to_rgb(pixels[1]);
        }
    }
}

std::array<float, 3> Database::hex_to_rgb(std::string hex){
    std::array<float, 3> rgb;
    rgb[0] = std::stoi(hex.substr(1, 2), nullptr, 16)/255.0f;
    rgb[1] = std::stoi(hex.substr(3, 2), nullptr, 16)/255.0f;
    rgb[2] = std::stoi(hex.substr(5, 2), nullptr, 16)/255.0f;

    return rgb;
}

//generates a configuration and sets internal variables to store quick info related to the game
//will choose best system to emulate automatically.
CPU::Config Database::gen_config(std::string filename){
    CPU::Config config;
    std::string hash = hash_bin(filename);
    std::cout << hash << std::endl;

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

    //set platform quirks
    set_platform_quirks(config, platform);

    //set game specific quirks
    set_game_quirks(config, game_rom);

    return config;
}
