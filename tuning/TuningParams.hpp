#ifndef TUNING_PARAMS_H
#define TUNING_PARAMS_H

#include "rocksdb/db.h"
#include <atomic>

void TuneParameters(rocksdb::DB* db, std::atomic<bool>& shouldExit);

#endif // TUNING_PARAMS_H
