#ifndef BITBOARD_H
#define BITBOARD_H

#include "Common.h"
#include "Judge.h"

class Bitboard {
private:
    /// The number of rows.
    int height;

    using BitboardType = __int128_t;
    BitboardType boards[2] = {0, 0};

    inline int getBoardIndex(Side side) {
        return 2 - side;
    }

    inline int getIndexOfPoint(int row, int column) {
        return column * (height + 1) + height - 1 - row;
    }

public:
    Bitboard() = default;
    Bitboard(int height, int width) : height(height) {
        myAssert(width * (height + 1) <= 128);
    }

    inline void set(int row, int column, Side side) {
        if (side == Side::NONE) {
            return;
        }
        int index = getIndexOfPoint(row, column);
        boards[getBoardIndex(side)] |= ((BitboardType)1 << index);
    }

    inline Side get(int row, int column) {
        int index = getIndexOfPoint(row, column);
        if (boards[getBoardIndex(Side::SELF)] & ((BitboardType)1 << index)) {
            return Side::SELF;
        } else if (boards[getBoardIndex(Side::OPPOSITE)] & ((BitboardType)1 << index)) {
            return Side::OPPOSITE;
        } else {
            return Side::NONE;
        }
    }

    inline void clear(int row, int column) {
        int index = getIndexOfPoint(row, column);
        BitboardType mask = ~((BitboardType)1 << index);
        boards[0] &= mask;
        boards[1] &= mask;
    }

    inline bool hasWon(Side side) {
        BitboardType board = boards[getBoardIndex(side)];
        BitboardType diag1 = board & (board >> height);
        BitboardType hori = board & (board >> (height + 1));
        BitboardType diag2 = board & (board >> (height + 2));
        BitboardType vert = board & (board >> 1);
        return ((diag1 & (diag1 >> (2 * height))) |
                (hori & (hori >> (2 * (height + 1)))) |
                (diag2 & (diag2 >> (2 * (height + 2)))) |
                (vert & (vert >> 2)));
    }
};

#endif
