//
// Created by kuiper on 2021/2/18.
//

#ifndef MY_LEVELDB_PORT_LIKELY_H
#define MY_LEVELDB_PORT_LIKELY_H

#if defined(__GNUC__) && __GNUC__ >= 4
#define LIKELY(x)   (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define LIKELY(x)   (x)
#define UNLIKELY(x) (x)
#endif

#endif //MY_LEVELDB_PORT_LIKELY_H
