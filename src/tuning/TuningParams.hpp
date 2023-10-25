#ifndef TUNING_PARAMS_H
#define TUNING_PARAMS_H

#include "rocksdb/db.h"
//#include "db_manager.hpp"
#include <atomic>

class TuneParameters {
public:
    TuneParameters(rocksdb::DB *db1);
    void tune_parameters(std::atomic<bool>& shouldExit);
private:
   extern rocksdb::DB* db;
};

#endif // TUNING_PARAMS_H
