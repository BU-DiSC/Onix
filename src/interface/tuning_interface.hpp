#include "workload_generator.hpp"

#ifndef TUNING_INTERFACE_H
#define TUNING_INTERFACE_H

class TuningInterface {
public:
    TuningInterface();
    static int tune_db(std::vector<std::string> values);
    static void restart_db_thread();
};
#endif