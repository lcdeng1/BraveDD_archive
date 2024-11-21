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
    inline NodeHandle getFreeNodeHandle(uint16_t lvl, const Node& node) {
        return chunks[lvl-1].getFreeNodeHandle(node);
    }

    /**
     *  Find the packed node (pointer) corresponding to a node handle
     */
    inline Node& getNodeFromHandle(uint16_t lvl, NodeHandle h) {
        return chunks[lvl-1].getNodeFromHandle(h);
    }


    /**
     *  Recycle a used node handle.
     *  The recycled handle can eventually be
     *  reused when returned by a call to
     *  getFreeNodeHandle().
     */
    void recycleNodeHandle(uint16_t lvl, NodeHandle h);

    /**
     *  Sweep a manager.
     *  For each node in it, check if it is marked or not.
     *  If marked, the mark bit(s) is cleared.
     *  If unmarked, the node is recycled.
     */
    inline void sweep(uint16_t lvl) { chunks[lvl-1].sweep(); }
    void sweep();

    inline uint32_t numUsed(uint16_t lvl) const { return chunks[lvl-1].numUsed(); }
    inline uint32_t numAlloc(uint16_t lvl) const { return chunks[lvl-1].numAlloc(); }

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    class SubManager {
        public:
            SubManager(Forest *f);
            ~SubManager();

            NodeHandle getFreeNodeHandle(const Node& node);

            Node& getNodeFromHandle(const NodeHandle h);

            void sweep();

            inline uint32_t numUsed() const {
                return PRIMES[sizeIndex] - numFrees;
            }
            inline uint32_t numAlloc() const {
                return firstUnalloc;
            }
        private:
        // ======================Helper Methods====================
            /// Expand the nodes to next size (if possible)
            void expand();
            /// Shrink the nodes to previous size
            void shrink();

        // ========================================================
            friend class NodeManager;
            Forest*     parent;         // Parent forest
            Node*       nodes;          // Actual node storage; the 1st slot (nodes[0]) will be used
            int         sizeIndex;      // Index of prime number for size
            uint32_t    firstUnalloc;   // Index of first unallocated slot
            uint32_t    freeList;       // Header of the list of unused slots
            uint32_t    numFrees;       // Number of free/unused slots
            uint32_t    recycled;       // Last recycled node index

    }; // class SubManager

    // ======================Helper Methods====================


    // ========================================================
    Forest* parent;        // Parent Forest
    SubManager* chunks;    // Chunks by levels

};

#endif