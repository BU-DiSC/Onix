#include "data_generator.hpp"
#include "random_generator.hpp"
#include <iostream>
#include <chrono>
#include <random>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "spdlog/spdlog.h"
#include <clipp.h>
#include <fstream>


using namespace std;
using namespace rocksdb;
using namespace clipp;
using namespace chrono;

extern std::string key_file_path;
extern rocksdb::DB *db;
DataGenerator::DataGenerator(rocksdb::DB *db1, std::string key_file_path1) {
    db = db1;
    key_file_path = key_file_path1;
}

rocksdb::Status DataGenerator::bulkLoader(int N, int key_size, int value_size) {

    WriteBatch batch;
    std::ofstream keyfile(key_file_path);
    std::string key;
    std::string value;
    std::pair<std::string, std::string> kv_pair;
//    spdlog::info("{} writing to keyfile", key_file_path);
    for (size_t i = 0; i < N; i++) {
        kv_pair = DataGenerator::gen_kv_pair(key_size, value_size);
        key = kv_pair.first;
        value = kv_pair.second;
//        spdlog::info("{} writing to keyfile", key);
        keyfile << key << '\n';
        batch.Put(key, value);
    }
    rocksdb::Status status = db->Write(WriteOptions(), &batch);
    keyfile.close();
    if (!status.ok()) {
        cerr << "Failed to perform bulk load: " << status.ToString() << endl;
    }
    keyfile.close();
    return status;
}

std::pair<std::string, std::string> DataGenerator::gen_kv_pair(int key_size, int value_size)
{
    RandomGenerator *random_generator = new RandomGenerator();
    std::string key = random_generator -> gen_key(key_size);
    std::string value = random_generator -> gen_value(value_size);

    return std::pair<std::string, std::string>(key, value);
}



