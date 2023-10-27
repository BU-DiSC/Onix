#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <future>
#include <fstream>
#include "clipp.h"
#include "data_generator.hpp"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "workload_generator.hpp"
#include "TuningParams.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <cstring>
#include <unistd.h>


std::string key_file_path =  "database/keyfile.txt";
std::shared_ptr<spdlog::logger> workloadLoggerThread;

int main(int argc, char * argv[]){
    using namespace clipp;

    try
            {
                workloadLoggerThread = spdlog::basic_logger_mt("workloadLoggerThread", "logs/workloadLoggerThread.txt");
            }
            catch (const spdlog::spdlog_ex &ex)
            {
                std::cout << "Log init failed: " << ex.what() << std::endl;
            }

    int N = 1000; // Number of records to prepopulate (adjust as needed)
    int empty_point_query_percentage = 25;
    int non_empty_point_query_percentage = 25;
    int range_query_percentage = 25;
    int write_query_percentage = 25;
    int num_queries = 1000; // Number of queries to perform for measuring performance
    int key_size = 10;
    int value_size = 100;
    std::string db_path = "database/mlosDb";

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

    rocksdb::DB* db = nullptr;
    rocksdb::Options options;
    options.create_if_missing = true;

    rocksdb::Status status = rocksdb::DB::Open(options, db_path, &db);

    if (!status.ok()) {
        std::cerr << "Failed to open database: " << status.ToString() << std::endl;
        spdlog::debug("Failed to open database: ",status.ToString());
        return 1;
    }

    DataGenerator *prePopulater = new DataGenerator(db,key_file_path);
    status = prePopulater->bulkLoader(N, key_size, value_size);
    if (!status.ok()) {
            spdlog::debug("Failed to bulk load database: ",status.ToString());
            return 1;
    }

      std::atomic<bool> shouldExit(false);
      TuneParameters t(db);

    // Start the parameter tuning thread
    std::thread parameter_tuning_thread(&TuneParameters::tune_parameters,&t,std::ref(shouldExit));

    WorkloadGenerator* run_workload = new WorkloadGenerator(db);
    for(int i=0;i<4;i++){
        run_workload -> GenerateWorkload(empty_point_query_percentage*num_queries*0.25*0.01,
        non_empty_point_query_percentage*num_queries*0.25*0.01,
        range_query_percentage*num_queries*0.25*0.01, write_query_percentage*num_queries*0.25*0.01,0, key_file_path);
    }
    while(true){
        for(int i=0;i<4;i++){
                run_workload -> GenerateWorkload(empty_point_query_percentage*num_queries*0.25*0.01,
                non_empty_point_query_percentage*num_queries*0.25*0.01,
                range_query_percentage*num_queries*0.25*0.01, 0, write_query_percentage*num_queries*0.25*0.01, key_file_path);
            }
    }
    spdlog::info("Done");
    parameter_tuning_thread.join();
    return 0;
}
