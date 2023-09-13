
#include <iostream>
#include <chrono>
#include <random>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include <clipp.h>


using namespace std;
using namespace rocksdb;
using namespace clipp;
using namespace chrono;



int main(int argc, char** argv) {
string dbPath;
size_t numRecords;

auto cli = (
    value("dbPath", dbPath),
    value("numRecords", numRecords)
);

if (!parse(argc, argv, cli)) {
cout << make_man_page(cli, argv[0]);
return 1;
}

DB* db;
Options options;
options.create_if_missing = true;
options.IncreaseParallelism();
options.OptimizeUniversalStyleCompaction();
Status status = DB::Open(options, dbPath, &db);
if (!status.ok()) {
cerr << "Failed to open or create the database: " << status.ToString() << endl;
return 1;
}
WriteBatch batch;
for (size_t i = 0; i < numRecords; i++) {
batch.Put(key, value);
}
status = db->Write(WriteOptions(), &batch);
if (!status.ok()) {
cerr << "Failed to perform bulk load: " << status.ToString() << endl;
}

return 0;
}
