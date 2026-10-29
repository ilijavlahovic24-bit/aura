// src/cli/main.cpp
#include "data/aurd.hpp"
#include <cstdio>

int main() {
    std::printf("aura: data header size = %zu, block entry = %zu\n",
                sizeof(aura::data::aurd::Header),
                sizeof(aura::data::aurd::BlockEntry));
    return 0;
}