#include "random_generator.hpp"
#include "spdlog/spdlog.h"

RandomGenerator::RandomGenerator() : generator(std::random_device{}()), distribution('A', 'Z') {}

std::string RandomGenerator::gen_key(int keySize) {
    std::string key;
    for (int i = 0; i < keySize; i++) {
        key += gen_char();
    }
    return key;
}


std::string RandomGenerator::gen_value(int value_size)
{
    std::string value;
    for (int i = 0; i < value_size; i++) {
        value += gen_char();
    }
    return value;
}

char RandomGenerator::gen_char() {
    return static_cast<char>(distribution(generator));
}