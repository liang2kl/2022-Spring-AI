#ifndef BITBOARD_PLAYER_H
#define BITBOARD_PLAYER_H

#include "Bitboard.h"
#include "GamePlayer.h"

class BitboardPlayer: public GamePlayer {
private:
    Bitboard board;
    const int *originalBoard;
    const int *originalTop;

    virtual inline void setBoard(int row, int column, Side side) {
        board.set(row, column, side);
    }
    virtual inline void unsetBoard(int row, int column) {
        board.clear(row, column);
    }
    virtual inline bool judgeIsWinning(int lastChoice, Side lastSide) {
        return board.hasWon(lastSide);
    }
    virtual void restoreBoardAndTop() {
        this->board = Bitboard(rowWidth, columnWidth);
        for (int i = 0; i < rowWidth; i++) {
            for (int j = 0; j < columnWidth; j++) {
                this->board.set(i, j, (Side)originalBoard[i * columnWidth + j]);
            }
        }
        for (int i = 0; i < columnWidth; i++) {
            topBuffer[i] = originalTop[i];
        }
    }

public:
    /// Check if the board size can be fitted into a single int128.
    static bool compatible(int rowWidth, int columnWidth) {
        return (rowWidth + 1) * columnWidth <= 128;
    }
    BitboardPlayer(const int *board, const int *top, int rowWidth,
                   int columnWidth, int forbiddenX, int forbiddenY)
        : GamePlayer(board, top, rowWidth, columnWidth, forbiddenX, forbiddenY) {
        this->originalBoard = board;
        this->originalTop = top;
        restoreBoardAndTop();
    }

private:
#ifdef DEBUG
    void printBoard() {
        print("Board\n");
        for (int row = 0; row < rowWidth; row++) {
            for (int column = 0; column < columnWidth; column++) {
                if (column == forbiddenY && row == forbiddenX) {
                    print("X ");
                } else {
                    print("%d ", board.get(row, column));
                }
            }
            print("\n");
        }
    }
#endif
};

#endif