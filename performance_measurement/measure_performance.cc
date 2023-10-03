#include <rocksdb/db.h>
#include <clipp.h>
#include <iostream>
#include <chrono>
#include <data_generator>



int main(int argc, char* argv[]) {
    using namespace clipp;

    int N = 10000;
    int empty_point_query_percentage = 25;
    int non_empty_point_query_percentage = 25;
    int range_query_percentage = 25;
    int write_query_percentage = 25;
    string path_to_db = "../database/mlos_db"

    auto cli = (
        option("--N") & value("N", N),
        option("--z0") & value("empty_percentage", empty_point_query_percentage),
        option("--z1") & value("non_empty_percentage", non_empty_point_query_percentage),
        option("--q") & value("range_percentage", range_query_percentage),
        option("--w") & value("write_percentage", write_query_percentage)
    );

    if (!parse(argc, argv, cli)) {
        std::cerr << make_man_page(cli, argv[0]) << std::endl;
        return 1;
    }

    rocksdb::DB* db;
    rocksdb::Options options;
    options.IncreaseParallelism();
    options.OptimizeUniversalStyleCompaction();
    options.create_if_missing = true;
    data_generator.prePopulate(N)


    auto start_time = std::chrono::steady_clock::now();

    for (int i = 0; i < num_queries; ++i) {
        x= //get random key
        std::string key = "key_" + std::to_string(x);
        std::string value;
        status = db->Get(rocksdb::ReadOptions(), key, &value);
        if (!status.ok()) {
            std::cerr << "Failed to retrieve record: " << status.ToString() << std::endl;
            return 1;
        }
    }

    auto end_time = std::chrono::steady_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    double point_query_latency = static_cast<double>(elapsed_time) / num_queries;
    double point_query_throughput = num_queries / (static_cast<double>(elapsed_time) / 1000); // queries per second

    // Print the measured performance metrics
    std::cout << "Point Query Latency: " << point_query_latency << " ms" << std::endl;
    std::cout << "Point Query Throughput: " << point_query_throughput << " queries/second" << std::endl;

    // Close the database when done
    delete db;

    return 0;
}
