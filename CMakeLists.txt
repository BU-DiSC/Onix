cmake_minimum_required(VERSION 3.16)
project(MyProject)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the path to your own modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(LZ4_DIR "/opt/homebrew/Cellar/lz4")



include(ExternalProject)

# Define the rocksdb_static_lib as an external project
ExternalProject_Add(rocksdb_static_lib
    SOURCE_DIR "${PROJECT_SOURCE_DIR}/rocksdb"
    INSTALL_COMMAND ""
    CONFIGURE_COMMAND ""
    USES_TERMINAL_CONFIGURE 1
    BUILD_COMMAND $(MAKE) -C ${PROJECT_SOURCE_DIR}/rocksdb DISABLE_WARNING_AS_ERROR=true static_lib
)

# Define rocksdb as an imported static library
add_library(rocksdb STATIC IMPORTED)
add_dependencies(rocksdb rocksdb_static_lib)
set_target_properties(rocksdb PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/rocksdb/librocksdb.a)
target_include_directories(rocksdb INTERFACE "${PROJECT_SOURCE_DIR}/rocksdb/include")


add_subdirectory(${PROJECT_SOURCE_DIR}/spdlog)

# Find the required packages
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
find_package(BZip2 REQUIRED)
# find_package(LZ4 REQUIRED)
# find_package(ZSTD REQUIRED)

# Find and configure RocksDB
# find_package(RocksDB REQUIRED)

# Include directories for your project
include_directories(
    ${CMAKE_SOURCE_DIR}/clipp/include
    ${CMAKE_SOURCE_DIR}/rocksdb/include
    ${CMAKE_SOURCE_DIR}/data
    ${CMAKE_SOURCE_DIR}/interface
    ${CMAKE_SOURCE_DIR}/tuning
)

# List your source files
set(SOURCES
    data/data_generator.cc
    data/random_generator.cc
    data/workload_generator.cc
    tuning/TuningParams.cc
    interface/tuning_interface.cc
)

# Create the executable
add_executable(rocksdb_performance ${SOURCES})
target_include_directories(rocksdb_performance PRIVATE /usr/local/include)
# Link required libraries
target_link_libraries(rocksdb_performance
    Threads::Threads
    ZLIB::ZLIB
    BZip2::BZip2
#     LZ4::LZ4
#     ZSTD::ZSTD
#     RocksDB::RocksDB
)

# target_link_libraries(database PUBLIC
#     rocksdb
#     spdlog
# )

add_executable(tuning_interface ${CMAKE_SOURCE_DIR}/interface/tuning_interface.cc)
target_link_libraries(tuning_interface database)