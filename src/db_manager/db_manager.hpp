//#ifndef DB_MANAGER_H
//#define DB_MANAGER_H
#pragma once
#include "rocksdb/db.h"
#include "rocksdb/db.h"

extern rocksdb::DB* db;
void initializeDatabase();


//#endif