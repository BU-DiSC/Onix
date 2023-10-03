#include "data_generator.hpp"
#include "random_generator.hpp"
#include <iostream>
#include <chrono>
#include <random>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include <clipp.h>


using namespace std;
using namespace rocksdb;
using namespace clipp;
using namespace chrono;

std::string key_file_path = "../database/keyfile.txt";

rocksdb::Status bulkLoader(int N, std::string path_to_db, int key_size, int value_size) {

    Status status = DB::Open(options, dbPath, &db);
    if (!status.ok()) {
        cerr << "Failed to open or create the database: " << status.ToString() << endl;
        return status;
    }
    WriteBatch batch;
    ofstream keyfile(key_file_path);
    for (size_t i = 0; i < N; i++) {
        key,value = gen_kv_pair(key_size, value_size);
        keyfile << key << '\n';
        batch.Put(key, value);
    }
    status = db->Write(WriteOptions(), &batch);
    if (!status.ok()) {
        cerr << "Failed to perform bulk load: " << status.ToString() << endl;
    }
    keyfile.close();
    return status;
}

std::pair<std::string, std::string> gen_kv_pair(int key_size, int value_size)
{
    RandomGenerator *random_generator = new RandomGenerator();
    std::string key = random_generator -> gen_key(key_size);
    std::string value = gen_value(value_size);

    return std::pair<std::string, std::string>(key, value);
}



