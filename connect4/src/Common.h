#ifndef COMMON_H
#define COMMON_H

#include <assert.h>

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#ifdef DEBUG
#define print(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define myAssert(e) assert(e)
#define debug(s) s
#else
#define print(fmt, ...) ;
#define myAssert(e) ;
#define debug(s) ;
#endif

const int MAX_ROWS = 12;
const int MAX_COLUMNS = 12;

enum Side {
    SELF = 2,
    OPPOSITE = 1,
    NONE = 0,
};

inline Side reversedSide(Side side) {
    return side == Side::SELF ? Side::OPPOSITE : Side::SELF;
}

#endif
