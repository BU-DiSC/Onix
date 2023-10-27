#include "TuningParams.hpp"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <fstream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

rocksdb::DB * db;
extern std::shared_ptr<spdlog::logger> tuningParamsLoggerThread;
TuneParameters::TuneParameters(rocksdb::DB * db1){
    db=db1;
}

void TuneParameters::tune_parameters(std::atomic<bool>& shouldExit) {
    while (!shouldExit) {

        int pipe_fd = -1;
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
                        // Parse the input from the pipe
                        char* token = std::strtok(buffer, "=");
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
                                spdlog::info("New parameter values {} {}", optionName, optionValue);
                                rocksdb::Status status = db->SetOptions({{optionName, optionValue}});  //tune parameters
                                spdlog::info("Tuning parameters...");
                            }
                        }
                    }

                    close(pipe_fd);
                }
}