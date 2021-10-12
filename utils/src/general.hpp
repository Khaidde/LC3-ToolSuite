#pragma once

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>

#define fatal(...)                             \
    char buff[0xFF];                           \
    snprintf(buff, sizeof(buff), __VA_ARGS__); \
    throw std::runtime_error(buff)

bool check_overflow(int32_t num, char bits);

std::string read_file(const char* filePath);
