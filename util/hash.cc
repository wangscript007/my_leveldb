//
// Created by kuiper on 2021/2/8.
//

#include "util/hash.h"
#include "util/coding.h"
#include <cstring>

#ifndef FALLTHROUGH_INTENDED
#define FALLTHROUGH_INTENDED do {} while(0)
#endif

namespace leveldb {


    uint32_t Hash(const char* data, size_t n, uint32_t seed) {
        // Similar to murmur hash
        const uint32_t m = 0xc6a4a793;
        const uint32_t r = 24;
        const char* limit = data + n;
        uint32_t h = seed ^ (n * m);

        // Pick up four bytes at a time
        while (data + 4 <= limit) {
            uint32_t w = DecodeFixed32(data);
            data += 4;
            h += w;
            h *= m;
            h ^= (h >> 16);
        }

        // Pick up remaining bytes
        switch (limit - data) {
            case 3:
                h += static_cast<uint8_t>(data[2]) << 16;
                FALLTHROUGH_INTENDED;
            case 2:
                h += static_cast<uint8_t>(data[1]) << 8;
                FALLTHROUGH_INTENDED;
            case 1:
                h += static_cast<uint8_t>(data[0]);
                h *= m;
                h ^= (h >> r);
                break;
        }
        return h;
    }

}
