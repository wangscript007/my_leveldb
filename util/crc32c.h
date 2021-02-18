//
// Created by kuiper on 2021/2/18.
//

#ifndef MY_LEVELDB_CRC32C_H
#define MY_LEVELDB_CRC32C_H

#include <cstddef>
#include <cstdint>

namespace leveldb {
    namespace crc32c {

        static constexpr uint32_t kMaskDelta = 0xa282ead8ul;

        uint32_t Extend(uint32_t init_crc, const char *data, size_t n);

        inline uint32_t Value(const char *data, size_t n) {
            return Extend(0, data, n);
        }

        // Return a masked representation of crc.
        //
        // Motivation: it is problematic to compute the CRC of a string that
        // contains embedded CRCs.  Therefore we recommend that CRCs stored
        // somewhere (e.g., in files) should be masked before being stored.
        inline uint32_t Mask(uint32_t crc) {
            // Rotate right by 15 bits and add a constant.
            return ((crc >> 15) | (crc << 17)) + kMaskDelta;
        }

        // Return the crc whose masked representation is masked_crc.
        inline uint32_t Unmask(uint32_t masked_crc) {
            uint32_t rot = masked_crc - kMaskDelta;
            return ((rot >> 17) | (rot << 15));
        }
    }
}

#endif //MY_LEVELDB_CRC32C_H
