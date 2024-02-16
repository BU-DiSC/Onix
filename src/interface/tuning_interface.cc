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
    int N = 1000; // Number of records to prepopulate
    std::atomic<bool> ex(false);
    try
    {
        workloadLoggerThread = spdlog::basic_logger_mt("workloadLoggerThread", "logs/workloadLoggerThread.txt");
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        spdlog::error("Workload Log init failed: {}",ex.what());
    }

    auto cli = (
        option("--N") & value("N", N),
        option("--empty-point-query-percentage") & value("empty_percentage", empty_point_query_percentage),
        option("--non-empty-point-query-percentage") & value("non_empty_percentage", non_empty_point_query_percentage),
        option("--range-query-percentage") & value("range_percentage", range_query_percentage),
        option("--write-query-percentage") & value("write_percentage", write_query_percentage),
        option("--num-queries") & value("num_queries", num_queries)
    );

    if (!parse(argc, argv, cli)) {
        return 1;
    }

    std::atomic<bool> shouldExit(false);
    TuneParameters t;
    Database_Handler dbh{N};

    // Start the parameter tuning thread
    std::thread parameter_tuning_thread(&TuneParameters::tune_parameters,&t,std::ref(shouldExit));
    std::thread workload_running_thread(&Database_Handler::run_workloads,empty_point_query_percentage,
    non_empty_point_query_percentage,range_query_percentage,write_query_percentage,num_queries);

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

//option 2 - kill thread
void TuningInterface::restart_db_thread(){
    spdlog::info("kill workload thread");
    workload_running_thread.std::thread::~thread();
    spdlog::info("restart workload thread");
    std::thread workload_running_thread(&Database_Handler::run_workloads,empty_point_query_percentage,
        non_empty_point_query_percentage,range_query_percentage,write_query_percentage,num_queries);
        workload_running_thread.detach();
        return;
}

int TuningInterface::tune_db(std::vector<std::string> values){
    int e = Database_Handler::TuneDB(values);
    if (e==-1){
        restart_db_thread();
    }
    return e;
}



