//
// Created by kuiper on 2021/2/4.
//

#ifndef MY_LEVELDB_EXPORT_H
#define MY_LEVELDB_EXPORT_H

#ifndef LEVELDB_EXPORT

#ifdef LEVELDB_SHARED_LIBRARY

#ifdef _WIN32

#if defined(LEVELDB_COMPILE_LIBRARY)
#define LEVELDB_EXPORT __declspec(dllexport)
#else
#define LEVELDB_EXPORT __declspec(dllimport)
#endif

#else  // NOT _WIN32

#ifdef LEVELDB_COMPILE_LIBRARY
#define LEVELDB_EXPORT  __attribute__((visibility("default")))
#else
#define LEVELDB_EXPORT
#endif

#endif

#else   // static lib
#define LEVELDB_EXPORT
#endif

#endif // LEVELDB_EXPORT
#endif // MY_LEVELDB_EXPORT_H
