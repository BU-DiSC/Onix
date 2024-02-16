#ifndef DATABASE_HANDLER_H
#define DATABASE_HANDLER_H

class Database_Handler {
public:
    Database_Handler(int N);
    static void restart_db();
    static void run_workloads(int empty_point_query_percentage,
                              int non_empty_point_query_percentage,
                              int range_query_percentage,
                              int write_query_percentage,
                              int num_queries);
    static int TuneDB(std::vector<std::string> keyValuePairs);
};
#endif