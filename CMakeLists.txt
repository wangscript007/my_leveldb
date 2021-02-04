cmake_minimum_required(VERSION 3.17)
project(my_leveldb)

set(CMAKE_CXX_STANDARD 17)

INCLUDE_DIRECTORIES(./include)

add_executable(my_leveldb main.cpp util/cache.cc)