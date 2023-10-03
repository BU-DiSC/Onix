#ifndef DATA_GENERATOR_H
#define DATA_GENERATOR_H

#include <string>
#include "rocksdb/db.h"
#include "rocksdb/options.h"

class DataGenerator {
public:
    DataGenerator(const std::string& path_to_db);

    rocksdb::Status bulkLoader(int N, int key_size, int value_size);

    std::pair<std::string, std::string> gen_kv_pair();

private:
    rocksdb::DB* db;
    rocksdb::Options options;
    std::string key_file_path;
};

#endif