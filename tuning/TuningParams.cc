#include "TuningParams.h"
#include "rocksdb/db.h"
#include "rocksdb/options.h"

void TuneParameters(rocksdb::DB* db, std::atomic<bool>& shouldExit) {
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

        rocksdb::Options rocksdb_opt;
        rocksdb::Status status = db->SetOptions({{optionName, std::to_string(optionValue)}});
        std::cout << "Tuning parameters..." << std::endl;
    }
}
