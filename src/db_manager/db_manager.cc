#include "db_manager.hpp"
#include <iostream>

rocksdb::DB* db = nullptr;

void initializeDatabase() {
    rocksdb::Options options;
    options.create_if_missing = true;
    rocksdb::Status status = rocksdb::DB::Open(options, "database/mlosDb", &db);
    if (!status.ok()) {
        std::cerr << "Failed to open database: " << status.ToString() << std::endl;
        exit(1);
    }
}
