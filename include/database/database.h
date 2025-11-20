#include <cpu/cpu.h>

#include <json.hpp>
using json = nlohmann::json;

class Database {
   public:
    Database(std::string dat_dir);

    CPU::Config gen_config(std::string filename);
    CPU::Config gen_platform_config(int platform);

   private:
    json sha1_hashes;
    json programs;
    json quirk_list;
    json platforms;

    int game_index;
    int platform;

    void set_platform_quirks(CPU::Config& config, int platform);
    void set_game_quirks(CPU::Config& config, json& game_rom);
    std::array<float, 3> hex_to_rgb(std::string hex);
};