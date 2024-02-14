#include <chrono>
#include <thread>
#include <atomic>
#include <future>
#include <fstream>
#include "clipp.h"
#include "TuningParams.hpp"
#include "database_handler.hpp"
#include "tuning_interface.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <cstring>
#include <unistd.h>



std::shared_ptr<spdlog::logger> workloadLoggerThread = nullptr;
std::shared_ptr<spdlog::logger> tuningParamsLoggerThread = nullptr;


std::thread parameter_tuning_thread;
std::thread workload_running_thread;
int empty_point_query_percentage = 25;
int non_empty_point_query_percentage = 25;
int range_query_percentage = 25;
int write_query_percentage = 25;
int num_queries = 1000; // Number of queries to perform for measuring performance


int main(int argc, char * argv[]){
    using namespace clipp;
    int N = 1000; // Number of records to prepopulate (adjust as needed)



        std::atomic<bool> ex(false);


    try
            {

                workloadLoggerThread = spdlog::basic_logger_mt("workloadLoggerThread", "logs/workloadLoggerThread.txt");
                //tuningParamsLoggerThread = spdlog::basic_logger_mt("tuningParamsLoggerThread", "logs/tuningParamsLoggerThread.txt");

            }
            catch (const spdlog::spdlog_ex &ex)
            {
                spdlog::error("Workload Log init failed: {}",ex.what());
            }



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
//        spdlog::debug(make_man_page(cli, argv[0]));
        return 1;
    }


      std::atomic<bool> shouldExit(false);
      TuneParameters t;
      spdlog::info("b4 dbh");
      Database_Handler dbh{N};

    // Start the parameter tuning thread
    std::thread parameter_tuning_thread(&TuneParameters::tune_parameters,&t,std::ref(shouldExit));
    std::thread workload_running_thread(&Database_Handler::run_workloads,empty_point_query_percentage,
    non_empty_point_query_percentage,range_query_percentage,write_query_percentage,num_queries);
//    std::thread workload_running_thread(&Database_Handler::run_workloads,&dbh,std::ref(empty_point_query_percentage),
//                                   std::ref(non_empty_point_query_percentage),std::ref(range_query_percentage),
//                                   std::ref(write_query_percentage),std::ref(num_queries));

    spdlog::info("after dbh");


    while(!ex){
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    spdlog::info("Done");
    parameter_tuning_thread.join();
    workload_running_thread.join();
    return 0;
}

TuningInterface::TuningInterface(){

}

int TuningInterface::tune_db(std::vector<std::string> values){
    int ep=Database_Handler::TuneDB(values);
    if (ep==-1){
        workload_running_thread.join();
        std::thread workload_running_thread(&Database_Handler::run_workloads,empty_point_query_percentage,
    non_empty_point_query_percentage,range_query_percentage,write_query_percentage,num_queries);
    }
   return ep; 
}



