cmake_minimum_required(VERSION 3.16)
project(mlos VERSION 1.0
                       LANGUAGES CXX
)

include(ExternalProject)
message(STATUS "CXX : ${CMAKE_CXX_COMPILER}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# =====================================================================================================================
# HEADER RocksDB
# =====================================================================================================================

ExternalProject_Add(rocksdb_static_lib
    SOURCE_DIR "${PROJECT_SOURCE_DIR}/libraries/rocksdb"
    INSTALL_COMMAND ""
    CONFIGURE_COMMAND ""
    USES_TERMINAL_CONFIGURE 1
    BUILD_COMMAND $(MAKE) -C ${PROJECT_SOURCE_DIR}/libraries/rocksdb DISABLE_WARNING_AS_ERROR=true static_lib
)

add_library(rocksdb STATIC IMPORTED)
add_dependencies(rocksdb rocksdb_static_lib)
set_target_properties(rocksdb PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/libraries/rocksdb/librocksdb.a)
target_include_directories(rocksdb INTERFACE "${PROJECT_SOURCE_DIR}/libraries/rocksdb/include")

# =====================================================================================================================
# HEADER Dependencies
# =====================================================================================================================
find_package(ZLIB REQUIRED)
find_package(BZip2 REQUIRED)
find_package(LZ4 REQUIRED)
find_package(Threads REQUIRED)
# find_package(uring REQUIRED)
find_package(zstd REQUIRED)
find_package(snappy REQUIRED)
find_package(Boost REQUIRED)
target_link_libraries(rocksdb INTERFACE
    BZip2::BZip2
    LZ4::LZ4
    Threads::Threads
    ZLIB::ZLIB
    zstd::zstd
    snappy::snappy
#     uring::uring
    boost:boost
    ${CMAKE_DL_LIBS}
)
include_directories(${Boost_INCLUDE_DIRS})
add_subdirectory(${PROJECT_SOURCE_DIR}/libraries/spdlog)
add_subdirectory(${PROJECT_SOURCE_DIR}/libraries/clipp)

# =====================================================================================================================
# HEADER DB Manager
# =====================================================================================================================
file(GLOB_RECURSE
    TEMPDB_SRCS
    CONFIGURE_DEPENDS
#     ${CMAKE_SOURCE_DIR}/src/*.cc
#     ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/db_manager/*.hpp
    ${CMAKE_SOURCE_DIR}/src/db_manager/*.cc
    ${CMAKE_SOURCE_DIR}/src/db_manager/*.cpp
)
add_library(tempdb OBJECT ${TEMPDB_SRCS})
target_include_directories(tempdb PUBLIC
    ${CMAKE_SOURCE_DIR}/src/db_manager
)

target_link_libraries(tempdb PUBLIC
    rocksdb
    spdlog
    clipp
    boost
)
# =====================================================================================================================
# Tools
# =====================================================================================================================

file(GLOB_RECURSE
    TOOLS_SRCS
    CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/data/*.cc
    ${CMAKE_SOURCE_DIR}/src/data/*.cpp
    ${CMAKE_SOURCE_DIR}/src/data/*.hpp
#     ${CMAKE_SOURCE_DIR}/src/tuning/*.cc
#     ${CMAKE_SOURCE_DIR}/src/tuning/*.hpp
)
add_library(tools OBJECT ${TOOLS_SRCS})
target_include_directories(tools PUBLIC
    ${CMAKE_SOURCE_DIR}/src/data
#     ${CMAKE_SOURCE_DIR}/src/tuning
)

target_link_libraries(tools PUBLIC
    rocksdb
    spdlog
    clipp
    boost
)
# Tools executables
add_executable(tuning_interface ${CMAKE_SOURCE_DIR}/src/interface/tuning_interface.cc)
target_link_libraries(tuning_interface tempdb tools ${Boost_LIBRARIES} )

add_executable(tuning_params ${CMAKE_SOURCE_DIR}/src/tuning/TuningParams.cc)
target_link_libraries(tuning_params tempdb)