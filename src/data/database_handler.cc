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
    spdlog::info("database handler num queries");
    std::cout<<empty_point_query_percentage<<non_empty_point_query_percentage<<range_query_percentage<<std::endl;
    WorkloadGenerator* run_workload= new WorkloadGenerator(db);
    while(flag){
        for(int i=0;i<4;i++){

            run_workload -> GenerateWorkload(empty_point_query_percentage*num_queries*0.25*0.01,
            non_empty_point_query_percentage*num_queries*0.25*0.01,
            range_query_percentage*num_queries*0.25*0.01, write_query_percentage*num_queries*0.25*0.01,0, key_file_path);
            //epochs++;
        }
        for(int i=0;i<4;i++){
                run_workload -> GenerateWorkload(empty_point_query_percentage*num_queries*0.25*0.01,
                non_empty_point_query_percentage*num_queries*0.25*0.01,
                range_query_percentage*num_queries*0.25*0.01, 0, write_query_percentage*num_queries*0.25*0.01, key_file_path);
                //epochs++;
        }
    }
}

void Database_Handler::restart_db(){

    spdlog::info("trying to restart db");
    flag=false;
    //run_workload();
    spdlog::info("signal db restart 1");
        rocksdb::CancelAllBackgroundWork(db,true);
        spdlog::info("signal db restart 2");
        std::this_thread::sleep_for (std::chrono::seconds(5));
        db->Close();
        spdlog::info("signal db restart 3");
        delete db;
        spdlog::info("signal db restart 4");
            //rocksdb::DB* new_db = nullptr;
            //rocksdb::Options options1;
             //options1.create_if_missing = false;
            rocksdb::Status status = rocksdb::DB::Open(options, db_path, &db);
            //db=new_db;
            spdlog::info("signal db restart 5");
            std::this_thread::sleep_for(std::chrono::seconds(10));
            flag=true;
            //std::thread workload_running_thread(TuningInterface::run_workloads);
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
            token = std::strtok(nullptr, "=");
            if (token != nullptr) {
                optionValue = token;

                // Check if 'exit' was entered
                if (optionName == "exit") {
                    break;
                }

                // Set the options in RocksDB
                spdlog::info("Tuning parameters start...");
                spdlog::info("New parameter values {} {}", optionName, optionValue);
                rocksdb::Status status = db->SetOptions({{optionName, optionValue}}); //tune parameters
                spdlog::info("Tuning parameters done...");
            }
        }
    }
    spdlog::info("Tuning parameters complete...");
    spdlog::info("epochs tuning params{}", epochs);
    int targetEpochs = epochs+10;
//    int x=0;
//    while (x<1 && epochs < initialEpochs + 10){
//        sleep(2);
//        x+=1;
//    }
   int e=epochs;
   spdlog::info("epochs tuning params {} {} {}", targetEpochs,epochs,e);
   if (epochs<targetEpochs){
    spdlog::info("going in restart db");
    restart_db();
    e=-1;
   }
   return e;
}
