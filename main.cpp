#include <iostream>
#include "leveldb/export.h"
#include "leveldb/slice.h"
#include "leveldb/cxx.h"
#include "leveldb/cache.h"
#include "leveldb/db.h"
#include "port/port_stdcxx.h"
#include "util/arena.h"
#include "util/histogram.h"
#include "leveldb/options.h"
#include "util/coding.h"

extern void testArena();
extern void testHistogram();

int main() {
    std::cout << "Hello, World!" << std::endl;

    leveldb::Slice s = "aaaaaa";
    std::cout << s.size() << std::endl;

    leveldb::Slice sb = "aaaaaaaaaa";

    std::cout << std::boolalpha << s.starts_with(sb) << std::endl;

    //leveldb::port::Mutex mutex;
    // mutex.Lock();
    // leveldb::port::CondVar condVar(&mutex);
    // condVar.Wait();

    leveldb::Options options;

    leveldb::ReadOptions readOptions;
    readOptions.verify_checksums = true;

    leveldb::WriteOptions writeOptions;
    writeOptions.sync = true;

    testArena();
    testHistogram();


    return 0;
}

 void testArena()
{
    leveldb::Arena a;
    for(int i = 0; i <100;i++) {
        a.Allocate(1);
    }
    std::cout << "total: " << a.MemoryUsage() << std::endl;
}

void testHistogram(){
    leveldb::Histogram histogram;

}
