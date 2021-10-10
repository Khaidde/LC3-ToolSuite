#include "general.hpp"

bool check_overflow(int32_t num, char bits) {
    int32_t lim = 1 << (bits - 1);
    return -lim > num || num >= lim;
}

std::string read_file(const char* filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        fatal("Cannot open file: %s\n", filePath);
    }

    std::streamsize fileSize = file.tellg();
    std::string res((std::size_t)fileSize, ' ');
    file.seekg(0);
    file.read(&res[0], fileSize);
    file.close();
    return res;
}
