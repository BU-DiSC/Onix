#ifndef TUNING_PARAMS_H
#define TUNING_PARAMS_H

#include "rocksdb/db.h"
#include <atomic>

class TuneParameters {
public:
    TuneParameters(rocksdb::DB * db);
    void tune_parameters(std::atomic<bool>& shouldExit);
};

#endif // TUNING_PARAMS_H
