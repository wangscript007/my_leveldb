#include <iostream>
#include "leveldb/export.h"
#include "leveldb/slice.h"
#include "leveldb/cxx.h"
#include "leveldb/cache.h"

int main() {
    std::cout << "Hello, World!" << std::endl;

    leveldb::Slice s = "aaaaaa";
    std::cout << s.size() << std::endl;

    leveldb::Slice sb = "aaaaaaaaaa";

    std::cout << std::boolalpha << s.starts_with(sb) << std::endl;
    return 0;
}
