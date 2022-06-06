#ifndef ARRAY_PLAYER_H
#define ARRAY_PLAYER_H

#include "GamePlayer.h"

class ArrayPlayer: public GamePlayer {
private:
    static inline int board[MAX_ROWS][MAX_COLUMNS] = {0};
    const int *originalBoard;
    const int *originalTop;

    virtual inline void setBoard(int row, int column, Side side) {
        ArrayPlayer::board[row][column] = side;
    }
    virtual inline void unsetBoard(int row, int column) {
        ArrayPlayer::board[row][column] = Side::NONE;
    }
    virtual bool judgeIsWinning(int lastChoice, Side lastSide) {
        int y = lastChoice;
        int x = topBuffer[y];
        // Should check if the point is actually the move we made.
        if (x == forbiddenX && y == forbiddenY) {
            x++;
        }

        if (lastSide == Side::SELF) {
            return Judge::machineWin(x, y, rowWidth, columnWidth, board);
        } else {
            return Judge::userWin(x, y, rowWidth, columnWidth, board);
        }
    }

    virtual void restoreBoardAndTop() {
        for (int i = 0; i < rowWidth; i++) {
            for (int j = 0; j < columnWidth; j++) {
                ArrayPlayer::board[i][j] = originalBoard[i * columnWidth + j];
            }
        }
        for (int i = 0; i < columnWidth; i++) {
            topBuffer[i] = originalTop[i];
        }
    }


public:
    ArrayPlayer(const int *board, const int *top, int rowWidth,
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
                    print("%d ", board[row][column]);
                }
            }
            print("\n");
        }
    }
#endif
};

#endif