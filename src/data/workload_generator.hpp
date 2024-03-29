#ifndef WORKLOAD_GENERATOR_H
#define WORKLOAD_GENERATOR_H

#include <string>
#include "rocksdb/db.h"

class WorkloadGenerator {

public:
    WorkloadGenerator(rocksdb::DB *db);
    void GenerateWorkload(
        double emptyPointQueries,
        double nonEmptyPointQueries,
        double rangeQueries,
        double writeQueries,
        double updateQueries,
        std::string key_file_path
    );
    std::vector<std::string> get_all_valid_keys(std::string key_file_path);
private:

    void append_valid_keys(std::string key_file_path, std::vector<std::string> & new_keys);
    int run_random_non_empty_reads(std::vector<std::string> existing_keys, rocksdb::DB * db, int num_queries);
    std::vector<std::string> generate_empty_keys(std::vector<std::string> existing_keys, int num_queries);
    int run_random_empty_reads(std::vector<std::string> existing_keys, rocksdb::DB * db, int num_queries);
    int run_range_reads(std::vector<std::string> existing_keys, rocksdb::DB * db, int num_queries);
    int run_random_inserts(std::string key_file_path, rocksdb::DB * db, int num_queries);
    int run_random_updates(std::vector<std::string> existing_keys, std::string key_file_path,rocksdb::DB * db, int num_queries);

};
#endif