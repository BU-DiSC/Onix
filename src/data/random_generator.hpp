#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H
#include <random>

class RandomGenerator {
public:
    RandomGenerator();

    std::string gen_key(int key_size);
    std::string gen_value(int value_size);
    char gen_char();

private:
    std::mt19937_64 generator;
    std::uniform_int_distribution<int> distribution;
};
#endif