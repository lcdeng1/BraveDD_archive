#ifndef BRAVE_DD_NODE_MANAGER_H
#define BRAVE_DD_NODE_MANAGER_H

#include "defines.h"
#include "node.h"

namespace BRAVE_DD {
    class Forest;
    class NodeManager;

    // I/O TBD
    // stats for performance measurement TBD
}

// ******************************************************************
// *                                                                *
// *                   Node Manager  class                          *
// *                                                                *
// ******************************************************************
class BRAVE_DD::NodeManager {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    NodeManager(Forest *f);
    ~NodeManager();

    /**
     *  Get an unused node handle.
     *  This is either a recycled one or
     *  the next one in the available pool
     *  (which will be expanded if necessary).
     *  Then fill it with the given unpacked node.
     */
    inline NodeHandle getFreeNodeHandle(const Level lvl, const Node& node) {
        NodeHandle handle = chunks[lvl-1].getFreeNodeHandle(node);
        // for sure number of used nodes +1
        numNodes++;
        // update peak
        if (numNodes > peak) peak = numNodes;
        return handle;
    }

    /**
     *  Find the node corresponding to a node handle
     */
    inline Node& getNodeFromHandle(const Level lvl, const NodeHandle h) {
        return chunks[lvl-1].getNodeFromHandle(h);
    }

    /**
     *  Recycle a used node handle.
     *  The recycled handle can eventually be
     *  reused when returned by a call to
     *  getFreeNodeHandle().
     */
    void recycleNodeHandle(Level lvl, NodeHandle h);

    /**
     *  Sweep a manager.
     *  For each node in it, check if it is marked or not.
     *  If marked, the mark bit(s) is cleared.
     *  If unmarked, the node is recycled.
     */
    void sweep(Level lvl);
    void sweep();

    void unmark(Level lvl);
    void unmark();

    inline uint32_t numUsed(Level lvl) const { return PRIMES[chunks[lvl-1].sizeIndex] - chunks[lvl-1].numFrees; }
    inline uint32_t numAlloc(Level lvl) const { return chunks[lvl-1].firstUnalloc; }
    inline uint32_t numMarked(Level lvl) const { return chunks[lvl-1].getNumMarked(); }
    inline uint32_t numPeakAlloc(Level lvl) const { return chunks[lvl-1].firstUnalloc - 1; }
    inline uint64_t numRealPeak() const { return peak; }
    inline void resetPeak() { peak = 0; }

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    class SubManager {
        public:
            SubManager(Forest *f);
            ~SubManager();

            void sweep();
        private:
        // ======================Helper Methods====================
            /// Get a free NodeHandle and fill it with a given node
            NodeHandle getFreeNodeHandle(const Node& node);
            /// Find the node corresponding to a node handle
            Node& getNodeFromHandle(const NodeHandle h);

            /// Get the number of marked nodes
            uint32_t getNumMarked() const;

            /// Expand the nodes to next size (if possible)
            void expand();
            /// Shrink the nodes to previous size
            void shrink();

        // ========================================================
            friend class NodeManager;
            Forest*                 parent;         // Parent forest
            std::vector<Node>       nodes;          // Actual node storage; the 1st slot (nodes[0]) will not be used
            int                     sizeIndex;      // Index of prime number for size
            uint32_t                firstUnalloc;   // Index of first unallocated slot
            uint32_t                freeList;       // Header of the list of unused slots
            uint32_t                numFrees;       // Number of free/unused slots
            uint32_t                recycled;       // Last recycled node index
            uint32_t                peak;           // Peak number of nodes
    }; // class SubManager

    // ======================Helper Methods====================


    // ========================================================
    Forest*                     parent;     // Parent Forest
    std::vector<SubManager>     chunks;     // Chunks by levels

    uint64_t                    numNodes;   // number of used nodes
    uint64_t                    peak;       // peak total numbe of used nodes
};

#endif