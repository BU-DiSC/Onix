#include "TuningParams.hpp"
#include "tuning_interface.hpp"
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
#include <unistd.h>
#include <sys/stat.h>


extern std::shared_ptr<spdlog::logger> tuningParamsLoggerThread;

TuneParameters::TuneParameters(){
    //db=db1;
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
int TuneParameters::signal_tune_db(std::vector<std::string> values){

    return TuningInterface::tune_db(values);

}
void TuneParameters::tune_parameters(std::atomic<bool>& shouldExit) {
//std::this_thread::sleep_for(std::chrono::seconds(15));
//signal_restart_db();

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
                               int epochs = signal_tune_db(keyValuePairs);
                                spdlog::info("Tuning parameters complete...");

                                std::string pipe_path = "passing_epochs";

                                    if (access(pipe_path.c_str(), F_OK) == -1) {
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
                                oss << epochs;
                                write(epochs_pipe_fd, oss.str().c_str(), oss.str().length());

                                spdlog::info("Reported epochs: {}", epochs);

                    }

                    close(pipe_fd);
                    close(epochs_pipe_fd);
                }
}




