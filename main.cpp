#include <iostream>
#include "leveldb/export.h"
#include "leveldb/slice.h"
#include "leveldb/cxx.h"
#include "leveldb/cache.h"
#include "leveldb/db.h"
#include "port/port_stdcxx.h"

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

    std::mutex mtx;
    std::condition_variable cv;

    std::unique_lock lock(mtx, std::adopt_lock);
    cv.wait(lock);

    std::cout << "I have change to be run" << std::endl;
    return 0;
}
