//#include "TuningParams.hpp"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include "spdlog/spdlog.h"
#include <fcntl.h> // for O_* constant
#include <unistd.h>
#include <atomic>
#include <boost_1_83_0/boost/interprocess/managed_mapped_file.hpp>
#include <boost_1_83_0/boost/interprocess/allocators/allocator.hpp>
#include <boost_1_83_0/boost/interprocess/containers/vector.hpp>
//#include "db_manager.hpp"

// for print error message
#include <string.h>
#include <errno.h>
class TuneParameters {
public:
    TuneParameters();
    void tune_parameters(std::atomic<bool>& shouldExit);
    void setDB();
private:
//   rocksdb::DB* db_;
};

//rocksdb::DB* db_ = nullptr;
//struct DatabaseHandle {
//    rocksdb::DB* db1;
//    bool set;
//    DbManager* db_manager;
//};

TuneParameters::TuneParameters() {
}


void tune_parameters(std::atomic<bool>& shouldExit) {
    while (!shouldExit) {

        // parameter tuning
        std::string optionName;
        std::string optionValue;

        // Listen for input from the console
        std::cout << "Enter option name ('exit' to quit): ";
        std::cin >> optionName;

        if (optionName == "exit") {
            shouldExit = true;
            break;
        }

        std::cout << "Enter option value: ";
        std::cin >> optionValue;
        rocksdb::Status status = db->SetOptions({{optionName, optionValue}});
        std::cout << "Tuning parameters..." << std::endl;

    }
}

void getDBUsingNamedPipe(){
std::atomic<bool> shouldExit(false);
//    std::string pipePath = "database/database_handler_pipe";
//        int pipeFileDescriptor = open(pipePath.c_str(), O_RDONLY);
//
//        if (pipeFileDescriptor == -1) {
//            std::cerr << "Failed to open the named pipe for reading." << std::endl;
//            return ;
//        }
//        std::cout<<"success - opened database handler"<<std::endl;
//        // Read the data from the named pipe
//        DatabaseHandle dbHandle;
//        ssize_t bytesRead = read(pipeFileDescriptor, &dbHandle, sizeof(dbHandle));
//        if (bytesRead <= 0) {
//            std::cerr << "Error reading from the named pipe." << std::endl;
//            return ;
//        }
//        TuneParameters t;

//        if (dbHandle.db1 && dynamic_cast<rocksdb::DB*>(dbHandle.db1)) {
//
//            std::cout << "got a valid db value" << std::endl;
//            db_=dbHandle.db1;
//        } else {
//            std::cerr << "Invalid or null dbHandle.db1 pointer." << std::endl;
//        }
//        if (db_ && dynamic_cast<rocksdb::DB*>(db_)) {

//            std::cout << "got a valid db_ value" << std::endl;
//            std::cout <<db << std::endl;

//            db_=dbHandle.db1;
//            DbManager db_manager = dbHandle.db_manager;
//            initializeDatabase();
            std::string optionName;
            std::string optionValue;

            // Listen for input from the console
            std::cout << "Enter option name ('exit' to quit): ";
            std::cin >> optionName;
            std::cout << "Enter option value: ";
            std::cin >> optionValue;

            rocksdb::Status status = db->SetOptions({{optionName, optionValue}});
            std::cout << "Tuning parameters..." << std::endl;
//        } else {
//            std::cerr << "Invalid or null db_ pointer." << std::endl;
//        }


//        t.tune_parameters(shouldExit);

        // Close the named pipe
//        close(pipeFileDescriptor);
}

void getDBUsingSharedMemory() {

            bip::managed_mapped_file segment(bip::open_or_create, "db_shared_memory", 65536);
                using SharedAllocator = bip::allocator<rocksdb::DB*, bip::managed_mapped_file::segment_manager>;
                using SharedDBVector = bip::vector<rocksdb::DB*, SharedAllocator>;

                // Retrieve the database handle from shared memory
                SharedDBVector* sharedDBs = segment.find<SharedDBVector>("sharedDBs").first;

                if (!sharedDBs) {
                    std::cerr << "Failed to access shared database handles." << std::endl;
                    return 1;
                }

                if (!sharedDBs->empty()) {
                    rocksdb::DB* db = (*sharedDBs)[0]; // Get the database handle

                    // Perform database operations using the db handle

                    // Continue with the rest of your code
                } else {
                    std::cerr << "No database handle found in shared memory." << std::endl;
                }

             // parameter tuning
             std::string optionName;
             std::string optionValue;

             // Listen for input from the console
             std::cout << "Enter option name ('exit' to quit): ";
             std::cin >> optionName;

             if (optionName == "exit") {
                 shouldExit = true;
                 break;
             }

             std::cout << "Enter option value: ";
             std::cin >> optionValue;
             rocksdb::Status status = db->SetOptions({{optionName, optionValue}});
             std::cout << "Tuning parameters..." << std::endl;

}

int main(int argc, char * argv[]){
    std::atomic<bool> shouldExit(false);
//    tune_parameters(shouldExit);
//    getDBUsingNamedPipe();
    getDBUsingSharedMemory();
    return 0;
}
