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
bool run_queries_flag = true;
extern std::shared_ptr<spdlog::logger> workloadLoggerThread;
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
        workloadLoggerThread->debug("Failed to open database: " + status.ToString() );
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
    while(run_queries_flag){
        for(int i=0;i<4;i++){
            run_workload -> GenerateWorkload(static_cast<double>(empty_point_query_percentage)*num_queries*0.25f*0.01,
            static_cast<double>(non_empty_point_query_percentage)*num_queries*0.25*0.01,
            static_cast<double>(range_query_percentage)*num_queries*0.25*0.01,
            static_cast<double>(write_query_percentage)*num_queries*0.25*0.01,0,
            key_file_path);
            std::string stats;
            db->GetProperty("rocksdb.stats",&stats);
             workloadLoggerThread->info("compaction stats {}",stats.c_str());
        }

    }
}

void Database_Handler::restart_db(){

    workloadLoggerThread->info("signal db restart 1");
    rocksdb::CancelAllBackgroundWork(db,true);
    workloadLoggerThread->info("signal db restart 2");
//    std::this_thread::sleep_for (std::chrono::seconds(5));
    db->Close();
//    std::this_thread::sleep_for (std::chrono::seconds(5));
    workloadLoggerThread->info("signal db restart 3");
    delete db;
    workloadLoggerThread->info("signal db restart 4");
    rocksdb::Status status = rocksdb::DB::Open(options, db_path, &db);
    workloadLoggerThread->info("signal db restart 5");
//    std::this_thread::sleep_for(std::chrono::seconds(5));
    workloadLoggerThread->info("signal db restart 6");

}
std::string trim(const std::string & source) {
    std::string s(source);
    s.erase(0,s.find_first_not_of(" \n\r\t"));
    s.erase(s.find_last_not_of(" \n\r\t")+1);
    return s;
}

int calculate_wait_time(){
    spdlog::info("waiting 2 milli secs");
    return 2;
    int wait_time = 10;
    num_of_bytes_written = options.statistics->getTickerCount(rocksdb::BYTES_WRITTEN);
    if (0.0000001*num_of_bytes_written>10) {
        wait_time=0.0000001*num_of_bytes_written;
    }

    workloadLoggerThread->info("wait time {}",wait_time);
    return wait_time;
}

void wait_for_compactions(){

    std::vector<std::string> keys = {rocksdb::DB::Properties::kMemTableFlushPending,
                                         rocksdb::DB::Properties::kNumRunningFlushes,
                                         rocksdb::DB::Properties::kCompactionPending,
                                         rocksdb::DB::Properties::kNumRunningCompactions};

        workloadLoggerThread->info("wait for compaction(%s): started\n",
                db->GetName().c_str());

        while (true) {
          bool retry = false;

          for (const auto& k : keys) {
            uint64_t v;
            if (!db->GetIntProperty(k, &v)) {
              workloadLoggerThread->info("wait for compaction({}): GetIntProperty({}) failed\n",
                      db->GetName().c_str(), k.c_str());
              exit(1);
            } else if (v > 0) {
              run_queries_flag = false;
              workloadLoggerThread->info("queries paused");
              workloadLoggerThread->info(
                      "wait for compaction({}): active({}). Sleep 2 milliseconds\n",
                      db->GetName().c_str(), k.c_str());
              std::this_thread::sleep_for(std::chrono::milliseconds(2));
              retry = true;
              break;
            }
          }

          if (!retry) {
            run_queries_flag = true;
            workloadLoggerThread->info("resume queries");
            workloadLoggerThread->info("wait for compaction {}: finished\n",
                    db->GetName().c_str());
            return;
          }
        }
}

int Database_Handler::TuneDB(std::vector<std::string> keyValuePairs){
    wait_for_compactions();
    std::string optionName;
    std::string optionValue;
    std::unordered_map<std::string, std::string> options_OptionsAPI;
    std::unordered_map<std::string, std::string> options_DbOptionsAPI;
    spdlog::info("len key value {}",keyValuePairs.size());
    for (const auto& keyValuePair : keyValuePairs) {
        spdlog::info("next pair {}", keyValuePair);
        size_t pos = keyValuePair.find('=');
        if (pos != std::string::npos) {
            std::string optionName = keyValuePair.substr(0, pos);
            std::string optionValue = keyValuePair.substr(pos + 1);
            trim(optionName);
            trim(optionValue);

            if (db_options_set.find(optionName) != db_options_set.end()) {
                options_DbOptionsAPI[optionName] = optionValue;
            } else {
                options_OptionsAPI[optionName] = optionValue;
            }

            spdlog::info("new parameter {} {}", optionName, optionValue);
            workloadLoggerThread->info("new parameter {} {}", optionName, optionValue);
        }
    }
    workloadLoggerThread->info("Tuning parameter...");
    try {
        if (!options_OptionsAPI.empty()){
            
            rocksdb::Status status1 = db->SetOptions(options_OptionsAPI);
            
            if (!status1.ok()) {
                spdlog::error("Failed to set options: {}", status1.ToString() );
                workloadLoggerThread->error("Failed to set options: {}", status1.ToString() );
            }
        }

        if (!options_DbOptionsAPI.empty()){
            rocksdb::Status status2 = db->SetDBOptions(options_DbOptionsAPI);
            if (!status2.ok()) {
                        spdlog::error("Failed to set db options:{} ",status2.ToString());
                        workloadLoggerThread->error("Failed to set db options:{} ",status2.ToString());
            }
        }
    } catch(const std::exception& e) {
        workloadLoggerThread->error("Exception caught: {}",e.what());
    } catch(...) {
        workloadLoggerThread->error("Unknown exception caught.");
    }

    spdlog::info("Tuning parameters complete...");
    workloadLoggerThread->info("Tuning parameters complete...");
    int targetEpochs = epochs+10;
    int x=0;
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    while (x<1000 && epochs < targetEpochs){
        workloadLoggerThread->info("waiting");
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        x+=1;
    }
   int e=std::max(epochs-1,0); //adjust indexing
   if (epochs<targetEpochs){ //database has hanged
    workloadLoggerThread->info("going in restart db");
       std::string stats;
        db->GetProperty("rocksdb.stats",&stats);
         workloadLoggerThread->info("compaction stats {}",stats.c_str());
//    restart_db();
    e=-epochs;
   }
   return e;
}

