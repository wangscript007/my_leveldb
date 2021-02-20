//
// Created by kuiper on 2021/2/20.
//

#ifndef MY_LEVELDB_COPYABLE_H
#define MY_LEVELDB_COPYABLE_H

#include <type_traits>

namespace leveldb {

    template<bool has_copy_constructor_and_copy_assign = true>
    struct Copyable {

    };

    template<>
    struct Copyable<false> {
        constexpr Copyable() = default;

        Copyable(const Copyable &) = delete;

        Copyable &operator=(const Copyable &) = delete;
    };
}


#endif //MY_LEVELDB_COPYABLE_H
