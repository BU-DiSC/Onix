#include "random_generator.hpp"

RandomGenerator::RandomGenerator() : generator(std::random_device{}()), distribution('A', 'Z') {}

std::string RandomGenerator::gen_key(size_t keySize) {
    std::string key;
    for (int i = 0; i < keySize; i++) {
        key += distribution(generator);
    }
    return key;
}


std::string RandomGenerator::gen_val(size_t value_size)
{
    std::string value = std::string(value_size, 'a');

    return value;
}