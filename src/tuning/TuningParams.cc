#include "TuningParams.hpp"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <sstream>
#include <vector>
#include <string>
#include <fstream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sys/stat.h>

rocksdb::DB * db;
extern std::string db_path;
extern int epochs;
extern std::shared_ptr<spdlog::logger> tuningParamsLoggerThread;

TuneParameters::TuneParameters(rocksdb::DB * db1){
    db=db1;
     try
                {

                    tuningParamsLoggerThread = spdlog::basic_logger_mt("tuningParamsLoggerThread", "logs/tuningParamsLoggerThread.txt");

                }
                catch (const spdlog::spdlog_ex &ex)
                {
                    spdlog::error("Workload Log init failed: {}",ex.what());
                }
}

std::vector<std::string> TuneParameters::parseKeyValuePairs(const std::string& input) {
    std::vector<std::string> keyValuePairs;
    std::istringstream iss(input);

    // Split the input into lines
    std::string line;
    while (std::getline(iss, line)) {
        keyValuePairs.push_back(line);
    }

    return keyValuePairs;
}
//void TuneParameters::restart_db(){
//    spdlog::info("restarting db");
//    rocksdb::Status x;
//    if (db != nullptr) {
//    x=db->Close();
//    spdlog::info("closing db");
//    }
//    if (!x.ok()) {
//        spdlog::debug("couldn't close rocksdb");
//        }
//        db=nullptr;
//        rocksdb::DB* new_db = nullptr;
//        rocksdb::Options options;
//        options.create_if_missing = false;
//        spdlog::info("options set");
//         rocksdb::Status status;
//        try {
//            status = rocksdb::DB::Open(options, db_path, &new_db);
//        }
//        catch (const std::exception& e) {
//            spdlog::error("Exception while opening RocksDB: {}", e.what());
//
//            if (!status.ok()) {
//                spdlog::debug("Failed to open database: " + status.ToString() );
//                return ;
//            }
//        }
//        new_db=db;
//        spdlog::info("restart complete");
//    }
void TuneParameters::restart_db(){
    spdlog::info("trying to restart db");
    rocksdb::Status close_status;
    if (db != nullptr) {
         close_status=db->Close();

    }
    if (!close_status.ok()) {
        spdlog::debug("couldn't close rocksdb instance");
    }
    db=nullptr;
    spdlog::info("closing old db complete");
    rocksdb::DB* new_db = nullptr;
    rocksdb::Options options;
    options.create_if_missing = false;
    spdlog::info("options for new db set");
    rocksdb::Status reopen_status;
    try {
       spdlog::info("opening new db at {}", db_path);
       reopen_status = rocksdb::DB::Open(options, db_path, &new_db);
    }
    catch (const std::exception& e) {
            spdlog::error("Exception while re opening RocksDB: {}", e.what());

            if (!reopen_status.ok()) {
                spdlog::debug("Failed to open database: " + reopen_status.ToString() );
                return ;
            }
    }
    db=new_db;
    spdlog::info("restart complete");
}
void TuneParameters::tune_parameters(std::atomic<bool>& shouldExit) {
    while (!shouldExit) {

        int pipe_fd = -1;
        int epochs_pipe_fd =-1;
        sleep(10);      //provide time to unlock the pipe
        while(pipe_fd == -1) {
            sleep(10);
            pipe_fd = open("passing_params_pipe", O_RDONLY);
            tuningParamsLoggerThread->error("Failed to open the named pipe. Error: {}", strerror(errno));
            sleep(25);
        }
            char buffer[128];
            ssize_t read_result;
            std::string optionName;
            std::string optionValue;
            while ((read_result = read(pipe_fd, buffer, sizeof(buffer))) > 0) {
                        buffer[read_result] = '\0';
                        std::vector<std::string> keyValuePairs = parseKeyValuePairs(buffer);

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
                                                shouldExit = true;
                                                break;
                                            }

                                            // Set the options in RocksDB
                                            spdlog::info("Tuning parameters start...");
                                            spdlog::info("New parameter values {} {}", optionName, optionValue);
                                            rocksdb::Status status = db->SetOptions({{optionName, optionValue}});
                                            //rocksdb::Status status = db->SetOptions({{"target_file_size_base", "1000"}});
                                            spdlog::info("Tuning parameters done...");
                                        }
                                    }
                                }
                                spdlog::info("Tuning parameters complete...");
                                spdlog::info("epochs tuning params{}", epochs);
                                int initialEpochs = epochs;
//                                int x=0;
//                                while (x<60 && epochs < initialEpochs + 10){
//                                    sleep(2);
//                                    x+=1;
//                                }
                               int e=epochs;
//                               if (epochs<initialEpochs+10){
                                restart_db();
                                e=-1;
                               //}

                                // Report the number of epochs through the pipe
                                std::string pipe_path = "passing_epochs";

                                    // Check if the pipe exists
                                    if (access(pipe_path.c_str(), F_OK) == -1) {
                                        // If it doesn't exist, create the pipe
                                        if (mkfifo(pipe_path.c_str(), 0666) == -1) {
                                            spdlog::error("Failed to create the passing_epochs pipe. Error: {}", strerror(errno));
                                            return ;
                                        }
                                    }

                                    // Open the pipe for writing
                                    int epochs_pipe_fd = open(pipe_path.c_str(), O_RDWR);
                                    if (epochs_pipe_fd == -1) {
                                        spdlog::error("Failed to open the passing_epochs pipe. Error: {}", strerror(errno));
                                        return ;
                                    }
                                std::ostringstream oss;
                                oss << e;
                                write(epochs_pipe_fd, oss.str().c_str(), oss.str().length());

                                spdlog::info("Reported epochs: {}", epochs);

                    }

                    close(pipe_fd);
                    close(epochs_pipe_fd);
                }
}




