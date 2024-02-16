#include "data_generator.hpp"
#include "database_handler.hpp"
#include "rocksdb/db.h"
#include "rocksdb/utilities/convenience.h"
#include "rocksdb/options.h"
#include "workload_generator.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
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
std::atomic<bool> flag(true);

Database_Handler::Database_Handler(int N){
    options.create_if_missing = true;
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
    while(flag){
        for(int i=0;i<4;i++){
            run_workload -> GenerateWorkload(static_cast<double>(empty_point_query_percentage)*num_queries*0.25f*0.01,
            static_cast<double>(non_empty_point_query_percentage)*num_queries*0.25*0.01,
            static_cast<double>(range_query_percentage)*num_queries*0.25*0.01,
            static_cast<double>(write_query_percentage)*num_queries*0.25*0.01,0,
            key_file_path);
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
int Database_Handler::TuneDB(std::vector<std::string> keyValuePairs){
    std::string optionName;
    std::string optionValue;
    for (const auto& keyValuePair : keyValuePairs) {
        // Parse each key=value pair
        char* token = std::strtok(const_cast<char*>(keyValuePair.c_str()), "=");
        if (token != nullptr) {
            optionName = token;
            if (optionName == "exit") {
                break;
            }
            optionValue = std::strtok(nullptr, "=");
            // Set the options in RocksDB
            spdlog::info("New parameter value {} {}", optionName, optionValue);
            rocksdb::Status status = db->SetOptions({{optionName, optionValue}}); //tune parameters
            spdlog::info("Tuning parameter done...");

        }
    }
    spdlog::info("Tuning parameters complete...");
    int targetEpochs = epochs+10;
    int starting_epoch=epochs;
    int x=0;

    std::this_thread::sleep_for(std::chrono::seconds(20));

    while (x<20 && epochs!=starting_epoch && epochs < targetEpochs){
        std::this_thread::sleep_for(std::chrono::seconds(5));
        x+=1;
    }

   int e=std::max(epochs-1,0); //adjust indexing
   if (epochs<targetEpochs){ //database has hanged
    spdlog::info("going in restart db");
//    restart_db();
    e=-1;
   }
   return e;
}

