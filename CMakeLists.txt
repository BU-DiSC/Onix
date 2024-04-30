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

ExternalProject_Add(speedb_static_lib
    SOURCE_DIR "${PROJECT_SOURCE_DIR}/libraries/speedb"
    INSTALL_COMMAND ""
    CONFIGURE_COMMAND ""
    USES_TERMINAL_CONFIGURE 1
    BUILD_COMMAND $(MAKE) -C ${PROJECT_SOURCE_DIR}/libraries/speedb DISABLE_WARNING_AS_ERROR=true static_lib
)

add_library(speedb STATIC IMPORTED)
add_dependencies(speedb speedb_static_lib)
set_target_properties(speedb PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/libraries/speedb/libspeedb.a)
target_include_directories(speedb INTERFACE "${PROJECT_SOURCE_DIR}/libraries/speedb/include")

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
target_link_libraries(speedb INTERFACE
    BZip2::BZip2
    LZ4::LZ4
    Threads::Threads
    ZLIB::ZLIB
    zstd::zstd
    snappy::snappy
#     uring::uring
    ${CMAKE_DL_LIBS}
)
add_subdirectory(${PROJECT_SOURCE_DIR}/libraries/spdlog)
add_subdirectory(${PROJECT_SOURCE_DIR}/libraries/clipp)


# =====================================================================================================================
# Tools
# =====================================================================================================================

file(GLOB_RECURSE
    TOOLS_SRCS
    CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/data/*.cc
    ${CMAKE_SOURCE_DIR}/src/data/*.cpp
    ${CMAKE_SOURCE_DIR}/src/data/*.hpp
    ${CMAKE_SOURCE_DIR}/src/tuning/*.cc
    ${CMAKE_SOURCE_DIR}/src/tuning/*.cpp
    ${CMAKE_SOURCE_DIR}/src/tuning/*.hpp
    ${CMAKE_SOURCE_DIR}/src/interface/*.hpp
)
add_library(tools OBJECT ${TOOLS_SRCS})
target_include_directories(tools PUBLIC
    ${CMAKE_SOURCE_DIR}/src/data
    ${CMAKE_SOURCE_DIR}/src/tuning
    ${CMAKE_SOURCE_DIR}/src/interface
)

target_link_libraries(tools PUBLIC
    speedb
    spdlog
    clipp
)
# Tools executables
add_executable(tuning_interface ${CMAKE_SOURCE_DIR}/src/interface/tuning_interface.cc)
target_link_libraries(tuning_interface tools)