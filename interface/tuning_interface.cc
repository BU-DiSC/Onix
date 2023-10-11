#include <rocksdb/utilities/options_util.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <future>
#include "clipp.h"
#include "../data/data_generator.hpp"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "../tuning/TuningParams.hpp"
#include "../data/workload_generator.hpp"

std::string key_file_path =  "../database/keyfile.txt";
int main(int argc, char* argv[]) {
    using namespace clipp;

    int N = 1000; // Number of records to prepopulate (adjust as needed)
    int empty_point_query_percentage = 25;
    int non_empty_point_query_percentage = 25;
    int range_query_percentage = 25;
    int write_query_percentage = 25;
    int num_queries = 1000; // Number of queries to perform for measuring performance
    int key_size = 10;
    int value_size = 100;
    std::string db_path = "../database/mlosDb";

    auto cli = (
        option("--N") & value("N", N),
        option("--empty-point-query-percentage") & value("empty_percentage", empty_point_query_percentage),
        option("--non-empty-point-query-percentage") & value("non_empty_percentage", non_empty_point_query_percentage),
        option("--range-query-percentage") & value("range_percentage", range_query_percentage),
        option("--write-query-percentage") & value("write_percentage", write_query_percentage),
        option("--num-queries") & value("num_queries", num_queries),
        option("--db_path") & value("db_path", db_path)

    );

    if (!parse(argc, argv, cli)) {
        std::cerr << make_man_page(cli, argv[0]) << std::endl;
        return 1;
    }

    rocksdb::DB* db;
    rocksdb::Options options;
    options.create_if_missing = true;

    // Open the database
    rocksdb::Status status = rocksdb::DB::Open(options, db_path, &db);

    if (!status.ok()) {
        std::cerr << "Failed to open database: " << status.ToString() << std::endl;
        return 1;
    }

    DataGenerator *prePopulater = new DataGenerator(db,key_file_path);
    status = prePopulater->bulkLoader(N, key_size, value_size);
    if (!status.ok()) {
            std::cerr << "Failed to bulk load database: " << status.ToString() << std::endl;
            return 1;
    }

     std::atomic<bool> shouldExit(false);
     TuneParameters t(db);

    // Start the parameter tuning thread
    std::thread parameter_tuning_thread(&TuneParameters::tune_parameters,&t,std::ref(shouldExit));

    WorkloadGenerator* run_workload = new WorkloadGenerator(db);
    for(int i=0;i<4;i++){
        run_workload -> GenerateWorkload(empty_point_query_percentage*num_queries*0.25,
        non_empty_point_query_percentage*num_queries*0.25,
        range_query_percentage*num_queries*0.25, write_query_percentage*num_queries*0.25, key_file_path);
    }

    parameter_tuning_thread.join();

    return 0;
}
