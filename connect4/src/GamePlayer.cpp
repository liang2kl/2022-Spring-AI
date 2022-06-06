#include "GamePlayer.h"

GamePlayer::GamePlayer(const int *board, const int *top, int rowWidth,
                       int columnWidth, int forbiddenX, int forbiddenY)
    : rowWidth(rowWidth), columnWidth(columnWidth), forbiddenX(forbiddenX), forbiddenY(forbiddenY) {
    // Initilize node pool, if not already.
    Node::initPool();

    // Setup root node
    root = Node::newNode();
    root->side = Side::OPPOSITE;
    // Mark the root as visited so that we can expand it
    // at the the first round.
    root->totalCount = 1;

    // The initialization of board and top array in done
    // in the constructor of derived class.
}

int GamePlayer::getBestMove(double timeLimit, int opponentLastMove) {
    time_t startTime = clock();

    print("The opponent's last move: column %d\n", opponentLastMove);
    debug(printBoard());

    // Check for good-quality choices
    deselectColumn(opponentLastMove);
    int suggedtedChoice = -1;
    SelectionQuality quality = detectBadChoice(
        opponentLastMove, Side::OPPOSITE, &suggedtedChoice);
    selectColumn(opponentLastMove, Side::OPPOSITE);

    if (quality == SelectionQuality::FORBIDDEN) {
        // A must-win situation for us. Happily return.
        print("Must win situation detected: column %d\n\n", suggedtedChoice);
        return suggedtedChoice;
    }

    // Check if we are in a critical situation BEFORE checking possible
    // good choices.
    int criticalColumn = getMustWinSituation(Side::OPPOSITE);
    if (criticalColumn != -1) {
        print("Critical situation detected: column %d\n\n", criticalColumn);
        return criticalColumn;
    }

    if (quality == SelectionQuality::BAD) {
        // A good choice for us.
        // But we still need to confirm that this move cannot create a must-win
        // situation for the rival.
        selectColumn(suggedtedChoice, Side::SELF);
        int criticalColumn = getMustWinSituation(Side::OPPOSITE);
        deselectColumn(suggedtedChoice);

        if (criticalColumn == -1) {
            print("Good situation detected: column %d\n\n", suggedtedChoice);
            return suggedtedChoice;
        } else {
            print("Good move abandoned: column %d\n", suggedtedChoice);
        }
    }

    // Simulate the game.
    debug(int i = 1);
    time_t limitInClocks = (time_t)(timeLimit * CLOCKS_PER_SEC);

    while (clock() - startTime < limitInClocks) {
        simulate();
        debug(i++);
    }

    debug(print("Total %d rounds\n", i));

#ifdef DEBUG
    // Benchmark results
    printEvaluationResult(clock() - startTime);
#endif

    // Find the best move.
    int bestChoice = -1;
    double bestScore = -1;

    int fallbackChoice = -1;
    double fallbackScore = -1;

    myAssert(root->numChildren > 0);

    for (int i = 0; i < root->numChildren; i++) {
        Node *child = Node::get(root->children[i]);
        double score = ucb(child, log(root->totalCount));
        // double score = (double)child->winCount / child->totalCount;
        print("%d: %f, %d\n", child->selection, score, child->totalCount);

        if (score > bestScore) {
            // Check if this move creates a must-win situation for the rival.
            SelectionQuality moveQuality = detectBadChoice(
                child->selection, Side::SELF);

            // Only allow to select if it cannot support the rival.
            if (moveQuality == SelectionQuality::FORBIDDEN) {
                print("Try to avoid a forbidden move: column %d\n", child->selection);
                continue;
            }

            // This move helps the rival.
            if (moveQuality == SelectionQuality::BAD) {
                // Record the choice in case we cannot get any better.
                print("Try to avoid a bad move: column %d\n", child->selection);
                if (score > fallbackScore) {
                    fallbackChoice = child->selection;
                    fallbackScore = score;
                }
                continue;
            }

            // The move is not too bad. Just take it.
            bestChoice = child->selection;
            bestScore = score;
        }
    }

    if (bestChoice >= 0) {
        print("Best choice: %d, score: %f\n", bestChoice, bestScore);
        if (bestScore < fallbackScore) {
            print("Warning: the choice is not the highest scored (%d: %f)\n",
                  fallbackChoice, fallbackScore);
        }
    } else if (fallbackChoice >= 0) {
        // Not too good, but we have to accecpt it.
        bestChoice = fallbackChoice;
        print("Fallback choice: %d, score: %f\n", fallbackChoice, fallbackScore);
    } else {
        // If all cases failed, we have to select one. We are doomed.
        bestChoice = Node::get(root->children[0])->selection;
        print("Worst but the only choice: %d\n", bestChoice);
    }

    // Reuse the node pool.
    Node::restorePool();

    print("\n");

    return bestChoice;
}

void GamePlayer::simulate() {
    Node *current = root;

    debug(int loopCount = 0);

    // Get to a leaf node
    debug(time_t start = clock());
    while (current->numChildren > 0) {
        int bestChild = -1;
        double bestScore = -1;

        // Find the best child
        double logN = log(current->totalCount) + 0.001;

        for (int i = 0; i < current->numChildren; i++) {
            int child = current->children[i];
            Node *node = Node::get(child);
            if (node->totalCount == 0) {
                bestChild = child;
                break;
            }
            double score = ucb(node, logN);
            if (bestChild == -1 || score > bestScore) {
                bestChild = child;
                bestScore = score;
            }
        }

        myAssert(bestChild != -1);
        current = Node::get(bestChild);

        // Play the move
        selectColumn(current->selection, current->side);

        debug(loopCount++);
        debug(myAssert(loopCount < 10000));
    }
    debug(traversalTime += clock() - start);

    if (current->totalCount != 0) {
        // We have checked this node before. Expand it.
        debug(start = clock());
        expand(current);
        debug(expandTime += clock() - start);
    }

    if (unlikely(current == root)) {
        return;
    }

    // Rollout.
    debug(start = clock());
    SimulationResult result = playCurrentGame(current->side,
        current->selection);
    debug(rolloutTime += clock() - start);

    // Backup
    debug(start = clock());
    backup(current, result);
    debug(backupTime += clock() - start);

    // Restore board and top
    debug(start = clock());
    restoreBoardAndTop();
    debug(restoreTime += clock() - start);
}

GamePlayer::SimulationResult GamePlayer::playCurrentGame(Side lastSide, int lastChoice) {
    for (;;) {
        // Check if the game is over.
        if (judgeIsWinning(lastChoice, lastSide)) {
            return lastSide == Side::SELF ? SimulationResult::WIN : SimulationResult::LOSE;
        } else if (Judge::isTie(columnWidth, topBuffer)) {
            return SimulationResult::DRAW;
        }

        debug(
            bool hasAvailableColumn = false;
            for (int i = 0; i < columnWidth; i++) {
                if (columnAvailable(i)) {
                    hasAvailableColumn = true;
                    break;
                }
            } if (!hasAvailableColumn) {
                print("No available column\n");
                printBoard();
                myAssert(false);
            });

        // The game is not over. Move forward to the next step.
        lastSide = reversedSide(lastSide);

        // Check for must-win situations.
        int mustWinColumn = -1;
        int midColumn = (columnWidth - 1) / 2;
        int sumScore = 0;
        int nextStepsCount = 0;
        
        for (int column = 0; column < columnWidth; column++) {
            if (columnAvailable(column)) {
                selectColumn(column, lastSide);
                bool won = judgeIsWinning(column, lastSide);
                deselectColumn(column);
                if (won) {
                    mustWinColumn = column;
                    break;
                }
                // Append to the selection list.
                nextSteps[nextStepsCount] = column;
                // Calculate and store the score. Simply prefer the centered ones.
                int distance = abs(column - midColumn);
                stepScores[nextStepsCount] = columnWidth - distance;
                sumScore += stepScores[nextStepsCount];
                nextStepsCount++;
            }
        }

        if (mustWinColumn != -1) {
            // You got a must-win column. That is the fate.
            return lastSide == SELF ? SimulationResult::WIN : SimulationResult::LOSE;
        } else {
            // Select a row randomly base on the scores.
            int choosenScore = rand() % sumScore;

            do {
                nextStepsCount--;
                choosenScore -= stepScores[nextStepsCount];
            } while (choosenScore > 0);

            lastChoice = nextSteps[nextStepsCount];

            myAssert(lastChoice >= 0);
            myAssert(columnAvailable(lastChoice));
        }

        // Play the move.
        selectColumn(lastChoice, lastSide);
    }
}

void GamePlayer::expand(Node *current) {
    Side rival = reversedSide(current->side);
    for (int column = 0; column < columnWidth; column++) {
        if (columnAvailable(column)) {
            // Next step can be played here. Create a new node.
            Node *node = Node::newNode();
            node->parent = current->self;
            node->selection = column;
            node->side = rival;
            current->children[current->numChildren] = node->self;
            current->numChildren++;
        }
    }
}

void GamePlayer::backup(Node *current, SimulationResult result) {
    bool userWin = result == SimulationResult::WIN;
    int increment = (current->side == Side::SELF && userWin) ||
                    (current->side == Side::OPPOSITE && !userWin) ? 1 : -1;
    if (result == SimulationResult::DRAW) {
        increment = 0;
    }

    debug(int loopCount = 0);
    while (current != root) {
        // Update the statistics
        current->winCount += increment;
        current->totalCount++;

        // Revert increment
        increment = -increment;

        // Trace back
        current = Node::get(current->parent);

        debug(loopCount++);
        debug(myAssert(loopCount < 10000));
    }

    // Update the root node
    root->totalCount++;
}

GamePlayer::SelectionQuality GamePlayer::detectBadChoice(
    int column, Side side, int *choice) {
    
    Side rival = reversedSide(side);

    // Check if this move creates a must-win situation for the rival.
    selectColumn(column, side);
    int mustWinColumn = getMustWinSituation(rival);
    deselectColumn(column);
    if (mustWinColumn >= 0) {
        // Must avoid this move.
        if (choice != nullptr) {
            *choice = mustWinColumn;
        }
        return SelectionQuality::FORBIDDEN;
    }

    SelectionQuality ret = SelectionQuality::OK;

    // Check if this move can create a possible double must-win situation for
    // the rival.
    selectColumn(column, side);
    for (int i = 0; i < columnWidth; i++) {
        if (columnAvailable(i)) {
            selectColumn(i, rival);
            int criticalColumn = getMustWinSituation(rival);

            if (criticalColumn >= 0) {
                int anotherCritical = getMustWinSituation(
                    rival, criticalColumn + 1);
                if (anotherCritical != -1) {
                    // Here's the one situation that we might lose
                    // if the rival is smart enough. Try to abandon this!
                    if (choice != nullptr) {
                        *choice = i;
                    }
                    ret = SelectionQuality::BAD;
                }
            }
            deselectColumn(i);

            if (ret == SelectionQuality::BAD) {
                break;
            }
        }
    }
    deselectColumn(column);

    return ret;
}

// Utility functions

inline void GamePlayer::selectColumn(int column, Side side) {
    topBuffer[column]--;

    myAssert(topBuffer[column] >= 0);

    // We should set it BEFORE checking the forbidden location!
    setBoard(topBuffer[column], column, side);

    // Deal with the forbidden point. Be careful!
    if (column == forbiddenY && topBuffer[column] - 1 == forbiddenX) {
        topBuffer[column]--;
    }
}

inline void GamePlayer::deselectColumn(int column) {
    // Deal with the forbidden point FIRST!
    if (column == forbiddenY && topBuffer[column] == forbiddenX) {
        topBuffer[column]++;
    }

    unsetBoard(topBuffer[column], column);
    topBuffer[column]++;
    myAssert(topBuffer[column] <= rowWidth);
}

inline bool GamePlayer::columnAvailable(int column) {
    // We need not check for the forbidden point as it
    // is dealt in selectColumn and deselectColumn.
    return topBuffer[column] > 0;
}

inline int GamePlayer::getMustWinSituation(Side side, int startColumn) {
    for (int column = startColumn; column < columnWidth; column++) {
        if (columnAvailable(column)) {
            selectColumn(column, side);
            bool won = judgeIsWinning(column, side);
            deselectColumn(column);
            if (won) {
                return column;
            }
        }
    }
    return -1;
}

inline double GamePlayer::ucb(Node *node, double logN) {
    return node->winCount / (double)node->totalCount +
           UCB_C * sqrt(2 * logN / node->totalCount);
}