#ifndef TUNING_PARAMS_H
#define TUNING_PARAMS_H

#include "rocksdb/db.h"
#include <atomic>

class TuneParameters {
public:
    TuneParameters();
    void tune_parameters(std::atomic<bool>& shouldExit);
    std::vector<std::string> parseKeyValuePairs(const std::string& input);
    int signal_tune_db(std::vector<std::string> values);
};

#endif // TUNING_PARAMS_H