#ifndef BRAVE_DD_NODE_H
#define BRAVE_DD_NODE_H

#include "defines.h"
#include "setting.h"
#include "hash_stream.h"
#include "edge.h"

namespace BRAVE_DD {
    static const uint32_t NODE_TYPE_MASK = (uint32_t)(0x01<<4);
    static const uint32_t NODE_LABEL_MASK = (uint32_t)((0x01<<27)-1)<<5;
    static const uint32_t LOW_TERMINAL_MASK = (uint32_t)((0x01<<2)-1)<<3;
    static const uint32_t HIGH_TERMINAL_MASK = (uint32_t)((0x01<<2)-1)<<1;
    static const uint32_t MARK_MASK = (uint32_t)(0x01);
    class Node;
}

// ******************************************************************
// *                                                                *
// *                         Node class                             *
// *                                                                *
// ******************************************************************
/** Node storage mechanism in a forest.
 *
 *  Every active node is stored in the following format:
 *  For 'Next' in unique table, 'Labels' of child edges, and 'Handles' of child nodes:
 *  Node has 5 uint32 info slots; Mxnode has 8 uint32 info slots.
 *      Common  --{ info[0]: Next node handle in the unique table.
 *      Labels  --{ info[1]: Bit 31 (MSB) ... Bit 28: child edge 0's rule.
 *                           Bit 27       ... Bit 24: child edge 1's rule.
 *                           Bit 23       ... Bit 20: child edge 2's rule (for Mxnode).
 *                           Bit 19       ... Bit 16: child edge 3's rule (for Mxnode).
 *                           Bit 15                 : child edge 1's complement.
 *                           Bit 14                 : child edge 2's complement.
 *                           Bit 13                 : child edge 3's complement.
 *                           Bit 12       ... Bit 11: child edge 0's swap bits (from and to).
 *                           Bit 10       ... Bit 9 : child edge 1's swap bits (from and to).
 *                           Bit 8        ... Bit 7 : child edge 2's swap bits.
 *                           Bit 6        ... Bit 5 : child edge 3's swap bits.
 *                           Bit 4        ... Bit 3 : 00:
 *                           Bit 2        ... Bit 1 : 00:
 *                           Bit 0                  : mark bit.
 *
 *    For Node:
 *      Child node
 *      handles --{ info[2]: child node 0's handle.     <---- for unused nodes, this is the next pointer in the free list.
 *                { info[3]: child node 1's handle.
 *      levels  --{ info[4]: Bit 31 (MSB) ... Bit 16: child node 0's level.
 *                           Bit 15       ... Bit 0 : child node 1's level.
 * 
 *    For Mxnode:
 *      Child node
 *      handles --{ info[2]: child node 00's handle.
 *                { info[3]: child node 01's handle.
 *                { info[4]: child node 10's handle.
 *                { info[5]: child node 11's handle.
 *      levels  --{ info[6]: Bit 31 (MSB) ... Bit 16: child node 00's level.
 *                           Bit 15       ... Bit 0 : child node 01's level.
 *                { info[7]: Bit 31 (MSB) ... Bit 16: child node 10's level.
 *                           Bit 15       ... Bit 0 : child node 11's level.
 * 
 *  For 'Values' of child edges if needed:
 *  Node has 1 (or 2 for LONG and DOUBLE) more slot for value if needed;
 *  Mxnode has 3 (or 6 for LONG and DOUBLE) more slots for values if needed.
 * 
 *  The construction can depend on the forest setting to further compress?
 * 
 */
class BRAVE_DD::Node {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    // construction by the forest setting
    Node(const ForestSetting& s) {
        int infoSize = s.nodeSize();
        info = (uint32_t*)malloc(infoSize * sizeof(uint32_t));
        for (int i=0; i<infoSize; i++) info[i] = 0;
    }
    Node(const int size) {
        info = (uint32_t*)malloc(size * sizeof(uint32_t));
        for (int i=0; i<size; i++) info[i] = 0;
    }
    ~Node() {
        free(info);
    }

    /// Methods =====================================================
    /**
     *  Get the next in unique table
     */
    inline NodeHandle getNext() const {return (NodeHandle)info[0];}

    /**
     *  Set the next for this node in unique table
     */
    inline void setNext(NodeHandle nxt) {info[0] = (uint32_t)nxt;}

    /**
     *  Check if this node is a matrix node for relation
     * 
     * @return true 
     * @return false 
     */
    inline bool isMxd() const { return info[1] & NODE_TYPE_MASK; }

    /**
     *  For this node, check if it is marked
     */
    inline bool isMarked() const {
        return (bool)(info[1] & MARK_MASK);
    }

    /**
     *  For this node, get the marks counting
     */
    inline int getMarks() const {
        return (info[1] & MARK_MASK);
    }

    /**
     *  Mark and increase 1 the count for this node
     *  Counting unchange if reach the max
     */
    inline void mark() {
        if ((uint32_t)getMarks()<MARK_MASK) info[1]++;
    }

    /**
     *  Unmark and decrease 1 the count for this node
     *  Counting unchange if reach 0
     */
    inline void unmark() {
        if (isMarked()) info[1]--;
    }

    /**
     *  Check if node is in use
     */
    inline bool isInUse() {
        // be careful! this may not detect all in-use node cases
        return info[1] || info[2] || info[3];
    }
    inline void recycle(uint32_t nextFree) {
        for (int i=0; i<4; i++) info[i] = 0;
        info[2] = nextFree;
    }
    inline NodeHandle nextFree() const {
        return (NodeHandle)info[2];
    }

    /**
     * Unpack and get the child edge's reduction rule 
     * 
     * @param child the index of the child edges: 0 ... 3
     * @return ReductionRule 
     */
    inline ReductionRule edgeRule(char child) const {
        if ((!(info[1] & NODE_TYPE_MASK) && child > 1) || child > 3) {
            // child index is out of the valid range, returns a value but throw error.
            throw error(ErrCode::INVALID_BOUND, __FILE__, __LINE__);
        }
        uint32_t EDGE_RULE_MASK = ((0x01 << 4) - 1) << (16 + 4 * (3 - child));
        return (ReductionRule)((info[1] & EDGE_RULE_MASK) >> (16 + 4 * (3 - child)));
    }

    inline void setEdgeRule(char child, ReductionRule rule) {
        if ((!(info[1] & NODE_TYPE_MASK) && child > 1) || child > 3) {
            // child index is out of the valid range, returns a value but throw error.
            throw error(ErrCode::INVALID_BOUND, __FILE__, __LINE__);
        }
        uint32_t EDGE_RULE_MASK = ((0x01 << 4) - 1) << (16 + 4 * (3 - child));
        info[1] &= ~EDGE_RULE_MASK;
        info[1] |= (uint32_t)rule << (16 + 4 * (3 - child));
    }

    /**
     * Unpack and get the child node handle
     * 
     * @param child the index of the child node: 0 ... 3
     * @return NodeHandle 
     */
    inline NodeHandle childNodeHandle(char child) const {
        if ((!(info[1] & NODE_TYPE_MASK) && child > 1) || child > 3) {
            // child index is out of the valid range, returns a value but throw error.
            throw error(ErrCode::INVALID_BOUND, __FILE__, __LINE__);
        }
        return (NodeHandle)info[2 + child];
    }

    inline void setChildNodeHandle(char child, NodeHandle handle) {
        if ((!(info[1] & NODE_TYPE_MASK) && child > 1) || child > 3) {
            // child index is out of the valid range, returns a value but throw error.
            throw error(ErrCode::INVALID_BOUND, __FILE__, __LINE__);
        }
        info[2 + child] = handle;
    }

    /**
     * Unpack and get the child node level
     * 
     * @param child the index of the child node: 0 ... 3
     * @return uint16_t 
     */
    inline uint16_t childNodeLevel(char child) const {
        if ((!(info[1] & NODE_TYPE_MASK) && child > 1) || child > 3) {
            // child index is out of the valid range, returns a value but throw error.
            throw error(ErrCode::INVALID_BOUND, __FILE__, __LINE__);
        }
        uint32_t NODE_LEVEL_MASK = ((0x01 << 16) - 1) << (16 * (1 - (child % 2)));
        return (uint16_t)((((info[1] & NODE_TYPE_MASK) ? info[6 + (child / 2)] : info[4]) 
                            & NODE_LEVEL_MASK) >> (16 * (1 - (child % 2))));
    }

    inline void setChildNodeLevel(char child, uint16_t lvl) {
        if ((!(info[1] & NODE_TYPE_MASK) && child > 1) || child > 3) {
            // child index is out of the valid range, returns a value but throw error.
            throw error(ErrCode::INVALID_BOUND, __FILE__, __LINE__);
        }
        uint32_t NODE_LEVEL_MASK = ((0x01 << 16) - 1) << (16 * (1 - (child % 2)));
        if (isMxd()) {
            info[6 + (child / 2)] &= ~NODE_LEVEL_MASK;
            info[6 + (child / 2)] |= (uint32_t)lvl << (16 * (1 - (child % 2)));
        } else {
            info[4] &= ~NODE_LEVEL_MASK;
            info[4] |= (uint32_t)lvl << (16 * (1 - (child % 2)));
        }
    }

    /**
     * Unpack and get the child edge's complement flag
     * 
     * @param child the index of the child edge: 0 ... 3
     * @return true 
     * @return false 
     */
    inline bool edgeComp(char child) const {
        if ((!(info[1] & NODE_TYPE_MASK) && child > 1) || child > 3) {
            // child index is out of the valid range, returns a value but throw error.
            throw error(ErrCode::INVALID_BOUND, __FILE__, __LINE__);
        }
        if (child == 0) return 0;
        return info[1] & (0x01 << (13 + (3 - child)));
    }

    inline void setEdgeComp(char child, bool comp) {
        if ((!(info[1] & NODE_TYPE_MASK) && child > 1) || child > 3 || child == 0) {
            // child index is out of the valid range, returns a value but throw error.
            throw error(ErrCode::INVALID_BOUND, __FILE__, __LINE__);
        }
        info[1] &= ~(0x01 << (13 + (3 - child)));
        info[1] |= comp << (13 + (3 - child));
    }

    /**
     * Unpack and get the child edge's swap flag
     * 
     * @param child the index of the child edge: 0 ... 3
     * @param swap  for Mxnode: 0 for from swap; 1 for to swap
     * @return true 
     * @return false 
     */
    inline bool edgeSwap(char child, bool swap) const {
        if ((!(info[1] & NODE_TYPE_MASK) && child > 1) || child > 3) {
            // child index is out of the valid range, returns a value but throw error.
            throw error(ErrCode::INVALID_BOUND, __FILE__, __LINE__);
        }
        if (!(info[1] & NODE_TYPE_MASK)) return info[1] & (0x01 << (10 + 2 * (1 - (child % 2))));
        return info[1] & (0x01 << (5 + 2 * (3 - child) + (1 - swap)));
    }

    inline void setEdgeSwap(char child, bool isTo, bool swap) {
        if ((!(info[1] & NODE_TYPE_MASK) && child > 1) || child > 3) {
            // child index is out of the valid range, returns a value but throw error.
            throw error(ErrCode::INVALID_BOUND, __FILE__, __LINE__);
        }
        if (isMxd()) {
            info[1] &= ~(0x01 << (5 + 2 * (3 - child) + (1 - isTo)));
            info[1] |= swap << (5 + 2 * (3 - child) + (1 - isTo));
        } else {
            info[1] &= ~(0x01 << (10 + 2 * (1 - (child % 2))));
            info[1] |= swap << (10 + 2 * (1 - (child % 2)));
        }
    }

    inline bool isEdgeTerminal(char child) const {
        if (childNodeLevel(child)>0) {
            //
        }
    }

    /**
     * Unpack and get the child edge's label
     * 
     * @param child the index of the child edge: 0 ... 3
     * @return EdgeLabel 
     */
    inline EdgeLabel edgeLabel(char child) const {
        EdgeLabel label = 0;
        packRule(label, this->edgeRule(child));
        packComp(label, this->edgeComp(child));
        packSwap(label, this->edgeSwap(child, 0));
        packSwapTo(label, this->edgeSwap(child, 1));
        return label;
    }

    inline void setEdgeLabel(char child, EdgeLabel label) {
        setEdgeRule(child, unpackRule(label));
        setEdgeComp(child, unpackComp(label));
        setEdgeSwap(child, 0, unpackSwap(label));
        setEdgeSwap(child, 1, unpackSwapTo(label));
    }

    /**
     * Unpack and get the child edge's value, fill it into 32 bits
     * 
     * @param child 
     * @return uint32_t 
     */
    inline uint32_t edgeValue32(char child) const {
        uint32_t val = 0;
        // TBD
        return val;
    }

    /**
     * Unpack and get the child edge's value, fill it into 64 bits
     * 
     * @param child 
     * @return uint64_t 
     */
    inline uint64_t edgeValue64(char child) const {
        uint64_t val = 0;
        // TBD
        return val;
    }

    /**
     * Hash this node
     * 
     * @return uint64_t 
     */
    inline uint64_t hash(int size) const {
        hash_stream hs;
        hs.start(0);
        // push info
        hs.push(info[1] >> 2);
        for (int i=2; i<size; i++) hs.push(info[i]);
        return (uint64_t)hs.finish64();
    }

    inline void assign(const Node& node, int size) {
        for (int i=0; i<size; i++) {
            info[i] = node.info[i];
        }
    }

    inline bool isEqual(const Node& node, int size) const {
        // labels
        if (((info[1] & NODE_LABEL_MASK) != (node.info[1] & NODE_LABEL_MASK))) return 0;
        // node handles and levels
        for (int i=2; i<size; i++) {
            if (info[i] != node.info[i]) return 0;
        }
        return 1;
    }

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    /// ============================================================
    friend class NodeManager;
    uint32_t* info;         // Next pointer, edge rules, edge flags, node handles, and levels
};


#endif