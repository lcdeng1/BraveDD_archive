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
    NodeHandle getFreeNodeHandle(Node node);

    /**
     *  Find the packed node (pointer) corresponding to a node handle
     */
    Node getNodeFromHandle(uint16_t lvl, NodeHandle h);


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
    void sweep(int varLvl);
    void sweep();


    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    class SubManager {
        public:
            SubManager(Forest *f);
            ~SubManager();

            uint32_t getFreeSlot();

            void sweep();
            
        private:
            /// Expand the hash table (if possible)
            void expand();

            /// Shrink the hash table
            void shrink();

            Forest* parent;                 // Parent forest
            std::vector<Node> nodes;  // Actual pack node storage
            int sizeIndex;                  // Index of prime number for size
            uint32_t recycled;              // Last recycled node index
            uint32_t firstUnalloc;          // Index of first unallocated slot
            uint32_t freeList;              // Header of the list of unused slots
            uint32_t numFrees;              // Number of unused slots
    }; // class SubManager

    // ======================Helper Methods====================
    inline NodeHandle constructHandle(int lvl, uint64_t index) {
        // check range
        #ifdef DCASSERTS_ON
            BRAVE_DD_DCASSERT(lvl <= ((0x01<<numLevelBits)-1) 
                        && index <= ((0x01<<(HANDLE_LENGTH-numLevelBits))-1));
        #endif
        return (lvl<<(HANDLE_LENGTH-numLevelBits)) | index;
    }


    // ========================================================
    Forest* parent;                     // Parent Forest
    std::vector<SubManager> chunks;     // Chunks by levels
    int numLevelBits;                   // number of level bits in node handle

};

#endif