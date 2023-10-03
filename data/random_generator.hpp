#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H

class RandomGenerator {
public:
    RandomGenerator();

    std::string gen_key();
    std::string gen_value();

private:
    std::mt19937_64 generator;
    std::uniform_int_distribution<char> distribution;
};