#include "TuningParams.hpp"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include <fstream>
#include <iostream>


rocksdb::DB * db;
TuneParameters::TuneParameters(rocksdb::DB * db1){
    db=db1;
}

void TuneParameters::tune_parameters(std::atomic<bool>& shouldExit) {
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