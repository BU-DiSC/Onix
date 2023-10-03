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
#include "TuningParams.hpp"


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

    auto cli = (
        option("--N") & value("N", N),
        option("--empty-point-query-percentage") & value("empty_percentage", empty_point_query_percentage),
        option("--non-empty-point-query-percentage") & value("non_empty_percentage", non_empty_point_query_percentage),
        option("--range-query-percentage") & value("range_percentage", range_query_percentage),
        option("--write-query-percentage") & value("write_percentage", write_query_percentage),
        option("--num-queries") & value("num_queries", num_queries)
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

    DataGenerator *prePopulater = new DataGenerator(db_path);
    status = prePopulater->bulkLoader(N, key_size, value_size);
    if (!status.ok()) {
            std::cerr << "Failed to bulk load database: " << status.ToString() << std::endl;
            return 1;
    }

     std::atomic<bool> shouldExit(false);
    // Start the parameter tuning thread
    std::thread parameter_tuning_thread(TuneDBParameters,db, std::ref(shouldExit));

    // Measure performance in the main thread
    PerformanceMetrics metrics = MeasurePerformance(db, num_queries);

    // Wait for the parameter tuning thread to finish
    parameter_tuning_thread.join();

    // Print the measured performance metrics
    std::cout << "Point Query Latency: " << metrics.latency << " ms" << std::endl;
    std::cout << "Point Query Throughput: " << metrics.throughput << " queries/second" << std::endl;
    std::cout << "Disk Space Utilization: " << metrics.disk_space << " bytes" << std::endl;

    // Close the database when done
//    delete db;

    return 0;
}
