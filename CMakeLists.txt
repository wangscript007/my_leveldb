cmake_minimum_required(VERSION 3.17)
project(my_leveldb)

# set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

INCLUDE_DIRECTORIES(./include)
INCLUDE_DIRECTORIES(./)

add_executable(my_leveldb
        main.cpp
        util/cache.cc
        util/coding.cc
        util/arena.cc
        util/histogram.cc
        util/options.cc
        util/comparator.cc
        util/status.cc
        util/logging.cc
        util/env.cc
        util/env_posix.cc
        util/crc32c.cc

        table/iterator.cc

        db/filename.cc
        db/log_writer.cc
        db/log_reader.cc
        db/memtable.cc
        db/dbformat.cc
        db/write_batch.cc
        #db/dumpfile.cc
        )