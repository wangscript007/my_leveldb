//
// Created by kuiper on 2021/2/7.
//

#ifndef MY_LEVELDB_NO_DESTRUCTOR_H
#define MY_LEVELDB_NO_DESTRUCTOR_H

#include <utility>
#include <type_traits>

namespace leveldb {

    template<typename InstanceType>
    class NoDestructor {
    public:

        template<typename... ConstructorArgTypes>
        explicit NoDestructor(ConstructorArgTypes &&... constructor_args) {
            static_assert(sizeof(instance_storage_) >= sizeof(InstanceType),
                          "instance_storage_ is not large enough to hold the instance.");

            static_assert(alignof(decltype(instance_storage_)) >= alignof(InstanceType),
                          "instance_storage_ does not meet the alignment requirement.");

            new(&instance_storage_) InstanceType(std::forward<ConstructorArgTypes>(constructor_args)...);
        }

        ~NoDestructor() = default;

        NoDestructor(const NoDestructor &) = delete;
        NoDestructor &operator=(const NoDestructor &) = delete;

        InstanceType *get() {
            return reinterpret_cast<InstanceType *>(&instance_storage_);
        }

    private:
        typename std::aligned_storage<sizeof(InstanceType),
                alignof(InstanceType)>::type instance_storage_;
    };

}

#endif //MY_LEVELDB_NO_DESTRUCTOR_H
