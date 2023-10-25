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
//#include "db_manager.hpp"
#include "spdlog/spdlog.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <cstring>
#include <unistd.h>
#include <boost/interprocess/managed_mapped_file.hpp>
#include "boost/interprocess/allocators/allocator.hpp"
#include "boost/interprocess/containers/vector.hpp"

//struct DatabaseHandle {
//    rocksdb::DB* db1;
//    bool set;
//    DbManager* db_manager;
//};
std::string key_file_path =  "database/keyfile.txt";
namespace bip = boost::interprocess;
//rocksdb::DB *db = nullptr;

//void setDBToTune(){
//    std::string pipePath = "database/database_handler_pipe";
//    mkfifo(pipePath.c_str(), 0666);    // Create the named pipe (if not exists)
//    int ret = open(pipePath.c_str(), O_WRONLY);
//    if (ret == -1) {
//            std::cerr << "Failed to open the named pipe for writing." << std::endl;
//            return;
//    }
//    DbManager *db_manager = new DbManager();
//    db_manager->initializeDatabase();
//    DatabaseHandle dbHandle;
//    dbHandle.db1 = db;
//    dbHandle.set = true;
//    dbHandle.db_manager = db_manager;
//    write(ret, &dbHandle, sizeof(dbHandle));
//    if (dbHandle.db1 && dynamic_cast<rocksdb::DB*>(dbHandle.db1)) {
//
//                std::cout << "got a valid dbHandle.db1 value tuning interface" << std::endl;
//            } else {
//                std::cerr << "Invalid or null dbHandle.db1 pointer. tuning interface" << std::endl;
//            }
//    if (db && dynamic_cast<rocksdb::DB*>(db)) {
//
//                    std::cout << "got a valid db value tuning interface" << std::endl;
//                } else {
//                    std::cerr << "Invalid or null db1 pointer. tuning interface" << std::endl;
//                }
//    close(ret);
//}

int main(int argc, char * argv[]){
    using namespace clipp;

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
//    if (db && dynamic_cast<rocksdb::DB*>(db)) {
//        std::cout << "got a valid db value tuning interface b4 db tune" << std::endl;
//    } else {
//        std::cerr << "Invalid or null db1 pointer. tuning interface b4 db tune" << std::endl;
//    }
//    setDBToTune();

//    initializeDatabase();
    // Create a memory-mapped file to store the database handle
        bip::managed_mapped_file segment(bip::open_or_create, "db_shared_memory", 65536);
        using SharedAllocator = bip::allocator<rocksdb::DB*, bip::managed_mapped_file::segment_manager>;
        using SharedDBVector = bip::vector<rocksdb::DB*, SharedAllocator>;

        SharedDBVector* sharedDBs = segment.construct<SharedDBVector>("sharedDBs")(segment.get_segment_manager());

        // Store the database handle in shared memory
        sharedDBs->push_back(db);
    DataGenerator *prePopulater = new DataGenerator(db,key_file_path);
    status = prePopulater->bulkLoader(N, key_size, value_size);
    if (!status.ok()) {
            spdlog::debug("Failed to bulk load database: ",status.ToString());
            return 1;
    }

//     std::atomic<bool> shouldExit(false);
//     TuneParameters *t= new TuneParameters();

    // Start the parameter tuning thread
//    std::thread parameter_tuning_thread(&TuneParameters::tune_parameters,&t,std::ref(shouldExit));

    WorkloadGenerator* run_workload = new WorkloadGenerator(db);
    for(int i=0;i<4;i++){
        run_workload -> GenerateWorkload(empty_point_query_percentage*num_queries*0.25*0.01,
        non_empty_point_query_percentage*num_queries*0.25*0.01,
        range_query_percentage*num_queries*0.25*0.01, write_query_percentage*num_queries*0.25*0.01, key_file_path);
    }
    while(true){
        for(int i=0;i<4;i++){
                run_workload -> GenerateWorkload(empty_point_query_percentage*num_queries*0.25*0.01,
                non_empty_point_query_percentage*num_queries*0.25*0.01,
                range_query_percentage*num_queries*0.25*0.01, 0, key_file_path);
            }
    }
    std::cout <<"Done" <<std::endl;
    spdlog::info("Done");
//    parameter_tuning_thread.join();
    return 0;
}
