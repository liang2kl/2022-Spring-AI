#ifndef GAME_PLAYER_H_
#define GAME_PLAYER_H_

#include <assert.h>

#include <cmath>
#include <cstring>
#include <ctime>
#include <random>

#include "Common.h"
#include "Judge.h"
#include "Bitboard.h"
#include "Node.h"

const static double UCB_C = 2;

class GamePlayer {
protected:
    /// The buffer used for intermediate calculation.
    inline static int topBuffer[MAX_COLUMNS] = {0};
    /// The buffer used to store available steps.
    inline static int nextSteps[MAX_COLUMNS] = {0};
    /// The buffer used to store the score of each step.
    inline static int stepScores[MAX_COLUMNS] = {0};

    int rowWidth = -1;
    int columnWidth = -1;
    int forbiddenX = -1;
    int forbiddenY = -1;

    /// The root node of the tree.
    Node *root = nullptr;

    // Pure virtual functions

    /// Place a chess.
    virtual void setBoard(int row, int column, Side side) = 0;
    /// Remove a chess.
    virtual void unsetBoard(int row, int column) = 0;
    /// Judge whether one side is winning.
    virtual bool judgeIsWinning(int lastChoice, Side lastSide) = 0;
    /// Restore to initial state
    virtual void restoreBoardAndTop() = 0;
#ifdef DEBUG
    virtual void printBoard() = 0;
#endif

#ifdef DEBUG
    // Time evaluation
    time_t traversalTime = 0;
    time_t rolloutTime = 0;
    time_t expandTime = 0;
    time_t restoreTime = 0;
    time_t backupTime = 0;

    void printEvaluationResult(time_t totalTime) {
        time_t recordedTime = traversalTime + rolloutTime + expandTime + restoreTime + backupTime;
        print("Recorded time: %.2f%%\n", recordedTime * 100.0 / totalTime);
        print("Traversal time: %.2f%%\n", traversalTime * 100.0 / totalTime);
        print("Rollout time: %.2f%%\n", rolloutTime * 100.0 / totalTime);
        print("Expand time: %.2f%%\n", expandTime * 100.0 / totalTime);
        print("Restore time: %.2f%%\n", restoreTime * 100.0 / totalTime);
        print("Backup time: %.2f%%\n", backupTime * 100.0 / totalTime);
    }
#endif

    void simulate();

    enum SimulationResult { WIN, LOSE, DRAW };

    /// Play current game randomly and return the result.
    SimulationResult playCurrentGame(Side lastSide, int lastChoice);

    void expand(Node *current);

    void backup(Node *current, SimulationResult result);

    enum SelectionQuality {
        /// The move will create a must-win situation for the rival.
        FORBIDDEN,
        /// The move will create a double must-win situation for the rival,
        /// if it is smart enough.
        BAD,
        /// No major immediate danger.
        OK
    };

    /// To check if a move results in immediate danger.
    SelectionQuality detectBadChoice(int column, Side side, int *choice = nullptr);

    void selectColumn(int column, Side side);

    void deselectColumn(int column);

    bool columnAvailable(int column);

    /// Check if there is a must-win situation for one side.
    ///
    /// @param side        The side of the move.
    /// @param startColumn The column to start searching, default 0.
    ///
    /// @return The first column starting from startColumn that is a
    ///         must-win column. -1 if there is no such column.
    int getMustWinSituation(Side side, int startColumn = 0);

    static double ucb(Node *node, double logN);

public:
    GamePlayer(const int *board, const int *top, int rowWidth,
               int columnWidth, int forbiddenX, int forbiddenY);

    /// Get the best move in given time limit.
    ///
    /// @param timeLimit The time limit in seconds.
    /// @return The best column to select.
    int getBestMove(double timeLimit, int opponentLastMove);
};

#endif