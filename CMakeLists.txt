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
        util/arena.cc
        util/histogram.cc)