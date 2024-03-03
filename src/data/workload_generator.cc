#include "workload_generator.hpp"
#include "random_generator.hpp"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/table.h"
#include "rocksdb/filter_policy.h"
#include "rocksdb/env.h"
#include "rocksdb/iostats_context.h"
#include "rocksdb/perf_context.h"
#include "zipf.hpp"
#include "data_generator.hpp"
#include "rocksdb/statistics.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/async.h"
#include "tuning_interface.hpp"
#include <clipp.h>
#include <fstream>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include <iterator>

#include "zipf.hpp"

#define PAGESIZE 4096

extern int key_size;
extern int value_size;
extern rocksdb::DB *db;
extern int epochs;
extern std::shared_ptr<spdlog::logger> workloadLoggerThread;
extern std::string db_path;

std::string performance_metrics_file_path = "performance/performance_metrics.csv";

WorkloadGenerator::WorkloadGenerator(rocksdb::DB *new_db){
    db=new_db;
}

void WorkloadGenerator::GenerateWorkload(
    double emptyPointQueries,
    double nonEmptyPointQueries,
    double rangeQueries,
    double writeQueries,
    double updateQueries,
    std::string key_file_path
) {
    //write performance metrics in a file
    std::ofstream metricsFile(performance_metrics_file_path,std::ios::app);
    if (!metricsFile.is_open()) {
        workloadLoggerThread->debug("Failed to open metrics file for writing.");
        return;
    }
    std::ifstream metricsFileRead(performance_metrics_file_path);
    if (metricsFileRead.is_open()) {
        epochs = std::count(
            std::istreambuf_iterator<char>(metricsFileRead),
            std::istreambuf_iterator<char>(),
            '\n'
        );
        metricsFileRead.close();
    }


    WorkloadGenerator workload_generator(db);

    int empty_read_duration = 0, read_duration = 0, range_duration = 0;
    int write_duration = 0, compact_duration = 0;
    std::vector<std::string> existing_keys;

    if ((nonEmptyPointQueries > 0) || (rangeQueries > 0 || emptyPointQueries > 0))
    {
        existing_keys = workload_generator.get_all_valid_keys(key_file_path);
    }

//    rocksdb_opt.statistics->Reset();
//    rocksdb::get_iostats_context()->Reset();
//    rocksdb::get_perf_context()->Reset();
//    if (writeQueries > 0)
//    {
//        write_duration = run_random_inserts(key_file_path, db, writeQueries);
//        if (write_duration==-1){
//            TuningInterface::restart_db_thread();
//        }
//    }
    if (writeQueries > 0)
    {
        write_duration = run_random_updates(existing_keys,key_file_path, db, writeQueries);
        if (write_duration==-1){
            TuningInterface::restart_db_thread();
        }
    }
    if (emptyPointQueries > 0)
    {
        empty_read_duration = run_random_empty_reads(existing_keys, db, emptyPointQueries);
    }

    if (nonEmptyPointQueries > 0)
    {
        read_duration = run_random_non_empty_reads(existing_keys, db, nonEmptyPointQueries);
    }

    if (rangeQueries > 0)
    {
        range_duration = run_range_reads(existing_keys, db, rangeQueries);
    }

    metricsFile << empty_read_duration << ","
                << read_duration << ","
                << range_duration << ","
                << write_duration << std::endl;
    workloadLoggerThread->info("metrics: {}, {}, {}, {}",empty_read_duration,read_duration,range_duration,write_duration);
    epochs++;
    metricsFile.close();
    return;

}

std::vector<std::string> WorkloadGenerator::get_all_valid_keys(std::string key_file_path)
{
    workloadLoggerThread->debug("Grabbing existing keys");
    std::vector<std::string> existing_keys;
    std::string key;
    std::ifstream key_file(key_file_path);

    if (key_file.is_open())
    {
        while (std::getline(key_file, key))
        {
            existing_keys.push_back(key);
        }
    }

    std::sort(existing_keys.begin(), existing_keys.end());

    return existing_keys;
}


void WorkloadGenerator::append_valid_keys(std::string key_file_path, std::vector<std::string> & new_keys)
{
    workloadLoggerThread->debug("Adding new keys to existing key file");
    std::ofstream key_file;
    key_file.open(key_file_path, std::ios::app);

    for (auto key : new_keys)
    {
        key_file << key << std::endl;
    }
    key_file.close();
}


int WorkloadGenerator::run_random_non_empty_reads(std::vector<std::string> existing_keys, rocksdb::DB * db, int num_queries)
{
    workloadLoggerThread->info("{} Non-Empty Reads", num_queries);
    rocksdb::Status status;

    std::string value;

    auto non_empty_read_start = std::chrono::high_resolution_clock::now();
    for (size_t read_count = 0; read_count < num_queries; read_count++)
    {
        int randomIndex = rand() % existing_keys.size();
        status = db->Get(rocksdb::ReadOptions(), existing_keys[randomIndex], &value);
    }
    auto non_empty_read_end = std::chrono::high_resolution_clock::now();
    auto non_empty_read_duration = std::chrono::duration_cast<std::chrono::milliseconds>(non_empty_read_end - non_empty_read_start);
    workloadLoggerThread->info("Non empty read time elapsed : {} ms", non_empty_read_duration.count());

    return non_empty_read_duration.count();
}

std::vector<std::string> WorkloadGenerator::generate_empty_keys(std::vector<std::string> existing_keys, int num_queries){
    RandomGenerator *randomGenerator = new RandomGenerator();
    std::string key;
    int ct=0;
    std::vector<std::string> empty_keys;
    while(ct<num_queries){
        key = randomGenerator->gen_key(key_size);
        if (std::find(existing_keys.begin(), existing_keys.end(), key) == existing_keys.end()) {
            empty_keys.push_back(key);
            ct++;
        }
    }
    return empty_keys;
}

int WorkloadGenerator::run_random_empty_reads(std::vector<std::string> existing_keys, rocksdb::DB * db, int num_queries)
{
    workloadLoggerThread->info("{} Empty Reads", num_queries);
    rocksdb::Status status;

    std::string value;

    std::vector<std::string> empty_keys = WorkloadGenerator::generate_empty_keys(existing_keys, num_queries);
    auto empty_read_start = std::chrono::high_resolution_clock::now();
    for (size_t read_count = 0; read_count < num_queries; read_count++)
    {
        status = db->Get(rocksdb::ReadOptions(), empty_keys[read_count], &value);
    }
    auto empty_read_end = std::chrono::high_resolution_clock::now();
    auto empty_read_duration = std::chrono::duration_cast<std::chrono::milliseconds>(empty_read_end - empty_read_start);
    workloadLoggerThread->info("Empty read time elapsed : {} ms", empty_read_duration.count());

    return empty_read_duration.count();
}


int WorkloadGenerator::run_range_reads(std::vector<std::string> existing_keys, rocksdb::DB * db, int num_queries)
{
    workloadLoggerThread->info("{} Range Queries", num_queries);
    rocksdb::ReadOptions read_opt;
    rocksdb::Status status;
    std::string lower_key, upper_key;
    int key_idx, valid_keys = 0;

    int key_hop = (PAGESIZE / key_size);

    std::string value;
    std::mt19937 engine;
    opencog::zipf_distribution<int, double> dist(existing_keys.size() - 1 - key_hop);


    read_opt.fill_cache = false;
    read_opt.total_order_seek = true;

    auto range_read_start = std::chrono::high_resolution_clock::now();
    for (size_t range_count = 0; range_count < num_queries; range_count++)
    {
        key_idx = dist(engine);
        lower_key = existing_keys[key_idx];
        upper_key = existing_keys[key_idx + key_hop];
        read_opt.iterate_upper_bound = new rocksdb::Slice(upper_key);
        rocksdb::Iterator * it = db->NewIterator(read_opt);
        for (it->Seek(rocksdb::Slice(lower_key)); it->Valid(); it->Next())
        {
            value = it->value().ToString();
            valid_keys++;
        }
        delete it;
    }
    auto range_read_end = std::chrono::high_resolution_clock::now();
    auto range_read_duration = std::chrono::duration_cast<std::chrono::milliseconds>(range_read_end - range_read_start);
    workloadLoggerThread->info("Range reads time elapsed : {} ms", range_read_duration.count());
    workloadLoggerThread->trace("Valid Keys {}", valid_keys);

    return range_read_duration.count();
}



int WorkloadGenerator::run_random_inserts(std::string key_file_path, rocksdb::DB * db, int num_queries)
{
    workloadLoggerThread->info("{} Write Queries", num_queries);
    rocksdb::WriteOptions write_opt;
    rocksdb::Status status;
    std::vector<std::string> new_keys;
    write_opt.no_slowdown = false; //> enabling this will make some insertions fail

    int max_writes_failed = num_queries * 0.1;
    int writes_failed = 0;

    workloadLoggerThread->debug("Writing {} key-value pairs", num_queries);
    DataGenerator *data_gen = new DataGenerator(db,key_file_path);
    auto start_write_time = std::chrono::high_resolution_clock::now();
    for (size_t write_idx = 0; write_idx < num_queries; write_idx++)
    {
        std::pair<std::string, std::string> entry = data_gen->gen_kv_pair(key_size,value_size);
        new_keys.push_back(entry.first);
        status = db->Put(write_opt, entry.first, entry.second);
        if (!status.ok())
        {
            workloadLoggerThread->warn("Unable to put key {}", write_idx);
            workloadLoggerThread->error("{}", status.ToString());
            writes_failed++;
            if (writes_failed > max_writes_failed)
            {
                workloadLoggerThread->error("10\% of total writes have failed, aborting");
//                return -1;
//                db->Close();
//                delete db;
//                exit(EXIT_FAILURE);
            }
        }
    }
    auto end_write_time = std::chrono::high_resolution_clock::now();
    auto write_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_write_time - start_write_time);
    workloadLoggerThread->info("writes time elapsed : {} ms", write_duration.count());
    workloadLoggerThread->debug("Flushing DB...");
    rocksdb::FlushOptions flush_opt;
    flush_opt.wait = true;
    flush_opt.allow_write_stall = true;

    db->Flush(flush_opt);
    append_valid_keys(key_file_path, new_keys);

    return write_duration.count();
}

int WorkloadGenerator::run_random_updates(std::vector<std::string> existing_keys,std::string key_file_path, rocksdb::DB * db, int num_queries)
{
    workloadLoggerThread->info("{} Write Queries", num_queries);
    rocksdb::WriteOptions write_opt;
    rocksdb::Status status;
    write_opt.no_slowdown = false; //> enabling this will make some insertions fail

    int max_writes_failed = num_queries * 0.1;
    int writes_failed = 0;

    workloadLoggerThread->debug("Writing {} key-value pairs", num_queries);
    DataGenerator *data_gen = new DataGenerator(db,key_file_path);
    auto start_write_time = std::chrono::high_resolution_clock::now();
    for (size_t write_idx = 0; write_idx < num_queries; write_idx++)
    {

            int randomIndex = rand() % existing_keys.size();
            std::pair<std::string, std::string> entry = data_gen->gen_kv_pair(key_size,value_size);
            status = db->Put(write_opt, existing_keys[randomIndex], entry.second);
        if (!status.ok())
        {
            workloadLoggerThread->warn("Unable to put key {}", write_idx);
            workloadLoggerThread->error("{}", status.ToString());
            writes_failed++;
            if (writes_failed > max_writes_failed)
            {
                workloadLoggerThread->error("10\% of total writes have failed, aborting");
//                return -1;
//                exit(EXIT_FAILURE);
            }
        }
    }
    auto end_write_time = std::chrono::high_resolution_clock::now();
    auto write_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_write_time - start_write_time);
    workloadLoggerThread->info("writes time elapsed : {} ms", write_duration.count());
    workloadLoggerThread->debug("Flushing DB...");
    rocksdb::FlushOptions flush_opt;
    flush_opt.wait = true;
    flush_opt.allow_write_stall = true;

    db->Flush(flush_opt);
    return write_duration.count();
}
