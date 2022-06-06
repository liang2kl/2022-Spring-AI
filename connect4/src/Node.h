#ifndef NODE_H
#define NODE_H

const static int NODE_POOL_SIZE = 10000000;

// Node of MCTree
struct Node {
    // Indices of children nodes
    int children[MAX_COLUMNS];
    /// Number of children
    int numChildren;
    /// Index of the parent node in the pool
    int parent;
    /// Index of the node in the pool
    int self;
    /// Selected column of this move
    int selection;
    // Number of winning cases
    int winCount;
    // Number of simulated cases
    int totalCount;
    // The side of this node
    Side side;

    inline void restoreSelf(int selfIndex) {
        numChildren = 0;
        parent = -1;
        self = selfIndex;
        selection = -1;
        winCount = 0;
        totalCount = 0;
    }

    /// Get a new node from the pool
    static inline Node *newNode() {
        myAssert(cursor < NODE_POOL_SIZE);
        Node *node = pool + cursor;
        node->restoreSelf(cursor);
        cursor++;
        return node;
    }

    /// Get the node by index in the pool
    static inline Node *get(int index) {
        myAssert(index >= 0);
        return &pool[index];
    }

    /// Initialize the node pool
    static inline void initPool() {
        if (unlikely(pool == nullptr)) {
            pool = new Node[NODE_POOL_SIZE];
            myAssert(pool);
        }
        cursor = 0;
    }

    static inline void restorePool() {
        print("Number of used nodes: %d\n", cursor);
        cursor = 0;
    }

private:
    inline static Node *pool = nullptr;
    inline static int cursor = 0;
};

#endif