#include "data_generator.hpp"
#include "database_handler.hpp"
#include "rocksdb/db.h"
#include "rocksdb/utilities/convenience.h"
#include "rocksdb/sst_file_manager.h"
#include "rocksdb/options.h"
#include "workload_generator.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <set>
#include <chrono>
#include <atomic>
#include <unistd.h>
#include <cstdlib>
#include <iostream>

std::string key_file_path =  "database/keyfile.txt";
std::string db_path = "database/mlosDb";
rocksdb::DB* db = nullptr;
rocksdb::Options options;
int epochs=0;
int key_size = 10;
int value_size = 100;
int num_of_bytes_written = 0;
std::set<std::string> db_options_set = {"max_open_files",
                                        "max_total_wal_size",
                                        "delete_obsolete_files_period_micros",
                                        "max_background_jobs",
                                        "max_subcompactions",
                                        "compaction_readahead_size",
                                        "writable_file_max_buffer_size",
                                        "delayed_write_rate",
                                        "avoid_flush_during_shutdown"};

Database_Handler::Database_Handler(int N){
    options.create_if_missing = true;
    options.statistics =  rocksdb::CreateDBStatistics();
    rocksdb::Status status = rocksdb::DB::Open(options, db_path, &db);

    if (!status.ok()) {
        spdlog::debug("Failed to open database: " + status.ToString() );
        return;
    }
    DataGenerator *prePopulater = new DataGenerator(db,key_file_path);
    status = prePopulater->bulkLoader(N, key_size, value_size);
    if (!status.ok()) {
            spdlog::debug("Failed to bulk load database: ",status.ToString());
            return;
    }
}


void Database_Handler::run_workloads(int empty_point_query_percentage,
    int non_empty_point_query_percentage, int range_query_percentage, int write_query_percentage, int num_queries ){
    WorkloadGenerator* run_workload= new WorkloadGenerator(db);
    while(true){
        for(int i=0;i<4;i++){
            run_workload -> GenerateWorkload(static_cast<double>(empty_point_query_percentage)*num_queries*0.25f*0.01,
            static_cast<double>(non_empty_point_query_percentage)*num_queries*0.25*0.01,
            static_cast<double>(range_query_percentage)*num_queries*0.25*0.01,
            static_cast<double>(write_query_percentage)*num_queries*0.25*0.01,0,
            key_file_path);
            num_of_bytes_written = options.statistics->getTickerCount(rocksdb::BYTES_WRITTEN);
            spdlog::info("statistics - number of blocks compressed {}",options.statistics->getTickerCount(rocksdb::NUMBER_BLOCK_COMPRESSED));
            spdlog::info("statistics - number of bytes written {}",num_of_bytes_written);
        }

    }
}

void Database_Handler::restart_db(){

    spdlog::info("signal db restart 1");
    rocksdb::CancelAllBackgroundWork(db,true);
    spdlog::info("signal db restart 2");
    std::this_thread::sleep_for (std::chrono::seconds(10));
    db->Close();
    std::this_thread::sleep_for (std::chrono::seconds(10));
    spdlog::info("signal db restart 3");
    delete db;
    spdlog::info("signal db restart 4");
    rocksdb::Status status = rocksdb::DB::Open(options, db_path, &db);
    spdlog::info("signal db restart 5");
    std::this_thread::sleep_for(std::chrono::seconds(10));
    spdlog::info("signal db restart 6");

}
std::string trim(const std::string & source) {
    std::string s(source);
    s.erase(0,s.find_first_not_of(" \n\r\t"));
    s.erase(s.find_last_not_of(" \n\r\t")+1);
    return s;
}

int calculate_wait_time(){
    int wait_time = 10;
    num_of_bytes_written = options.statistics->getTickerCount(rocksdb::BYTES_WRITTEN);
    if (num_of_bytes_written>0) {
        wait_time=0.0001*num_of_bytes_written;
    }
    spdlog::info("wait time {}",wait_time);
    return wait_time;
}
int Database_Handler::TuneDB(std::vector<std::string> keyValuePairs){
    std::string optionName;
    std::string optionValue;
    std::unordered_map<std::string, std::string> options_OptionsAPI;
    std::unordered_map<std::string, std::string> options_DbOptionsAPI;
    for (const auto& keyValuePair : keyValuePairs) {
        // Parse each key=value pair
        char* token = std::strtok(const_cast<char*>(keyValuePair.c_str()), "=");
        if (token != nullptr) {
            optionName = token;
            if (optionName == "exit") {
                break;
            }
            trim(optionName);
            optionValue = std::strtok(nullptr, "=");
            trim(optionValue);

            if (db_options_set.find(optionName) != db_options_set.end()){
                options_DbOptionsAPI[optionName] = optionValue;
            }
            else{
                options_OptionsAPI[optionName] = optionValue;

            }

            spdlog::info("new parameter {} {}",optionName,optionValue);

        }
    }
    spdlog::info("Tuning parameter...");
    try {
//        db->SetOptions({{optionName, optionValue},{optionName,optionValue}});
        rocksdb::Status status1 = db->SetOptions(options_OptionsAPI);

        if (!status1.ok()) {
                            std::cerr << "Failed to set options: " << status1.ToString() << std::endl;
                        }
        rocksdb::Status status2 = db->SetDBOptions(options_DbOptionsAPI);
        if (!status2.ok()) {
                    std::cerr << "Failed to set db options: " << status2.ToString() << std::endl;
                }
    } catch(const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    } catch(...) {
        std::cerr << "Unknown exception caught." << std::endl;
    }

    spdlog::info("Tuning parameters complete...");
    int targetEpochs = epochs+10;
    int x=0;

    std::this_thread::sleep_for(std::chrono::seconds(calculate_wait_time()));

    while (x<20 && epochs < targetEpochs){
        std::this_thread::sleep_for(std::chrono::seconds(calculate_wait_time()));
        x+=1;
    }
   int e=std::max(epochs-1,0); //adjust indexing
   if (epochs<targetEpochs){ //database has hanged
    spdlog::info("going in restart db");
//    restart_db();
    e=epochs*-1;
   }
   return e;
}

