#ifndef BRAVE_DD_NODE_H
#define BRAVE_DD_NODE_H

#include "defines.h"
#include "setting.h"
#include "hash_stream.h"
#include "edge.h"

namespace BRAVE_DD {
    static const uint32_t NODE_TYPE_MASK = (uint32_t)(0x01<<4);
    static const uint32_t MARK_MASK = (uint32_t)((0x01<<4)-1);
    class Node;
}

// ******************************************************************
// *                                                                *
// *                     Packed Node class                          *
// *                                                                *
// ******************************************************************
/** Packed node storage mechanism in a forest.
 *
 *  Every active node is stored in the following format:
 *  Node has 5 uint32 info slots; Mxnode has 8 uint32 info slots.
 *      Common  --{ info[0]: Next node handle in the unique table.
 *      Labels  --{ info[1]: Bit 31 (MSB) ... Bit 28: child edge 0's rule.
 *                           Bit 27       ... Bit 24: child edge 1's rule.
 *                           Bit 23       ... Bit 20: child edge 10's rule (for Mxnode).
 *                           Bit 19       ... Bit 16: child edge 11's rule (for Mxnode).
 *                           Bit 15                 : child edge 1's complement.
 *                           Bit 14                 : child edge 10's complement.
 *                           Bit 13                 : child edge 11's complement.
 *                           Bit 12       ... Bit 11: child edge 0's swap bits (from and to).
 *                           Bit 10       ... Bit 9 : child edge 1's swap bits (from and to).
 *                           Bit 8        ... Bit 7 : child edge 10's swap bits.
 *                           Bit 6        ... Bit 5 : child edge 11's swap bits.
 *                           Bit 4                  : 0: for Node; 1: for Mxnode.
 *                           Bit 3        ... Bit 0 : mark bits.
 *    For Node:
 *      Child node
 *      handles --{ info[2]: child node 0's handle.
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
 *  Node has 1 uint64 value slot; Mxnode has 3 uint64 value slots.
 * 
 *  The construction can depend on the forest setting to further compress?
 * 
 */
class BRAVE_DD::Node {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    Node() {}
    // construction by the forest setting
    Node(const ForestSetting& s) {
        bool isRelation = s.isRelation();
        int infoSize;
        /* slots for values.
            Note: for finite range, only logN bits needed: further compress TBD.
        */
        ValueType valType = s.getValType();
        if (valType==LONG || valType==DOUBLE) {
            infoSize = isRelation ? 8+3*2 : 5+2;
        } else if (valType==INT || valType==FLOAT) {
            infoSize = isRelation ? 8+3 : 5+1;
        } else {
            infoSize = isRelation ? 8 : 5;
        }
        info = (uint32_t*)malloc(infoSize * sizeof(uint32_t));
        for (int i=0; i<infoSize; i++) {
            info[i] = 0;
        }
        if (isRelation) info[1] |= NODE_TYPE_MASK;
    }
    ~Node() {
        for (int i=0; info[i]; i++) info[i] = 0;
        free(info);
        for (int i=0; values[i]; i++) values[i] = 0;
        free(values);
    }

    /// Methods =====================================================
    /**
     *  Get the next in unique table
     */
    inline NodeHandle getPackedNext() const {return (NodeHandle)info[0];}

    /**
     *  Set the next for this packed node in unique table
     */
    inline void setPackedNext(NodeHandle nxt) {info[0] = (uint32_t)nxt;}

    inline bool isMxd() const { return info[1] & NODE_TYPE_MASK; }

    /**
     *  For this packed node, is it marked?
     *  Marked if the mark counting greater than the given value
     * 
     */
    inline bool isPackedMarked(uint16_t val) const {
        return (bool)((uint16_t)(info[1] & MARK_MASK) > val);
    }

    /**
     *  Get the mark count for this packed node
     * 
     */
    inline uint16_t getPackedMarks() const {
        return (uint16_t)(info[1] & MARK_MASK);
    }

    /**
     *  Mark and increase 1 the count for this packed node
     *  Counting unchange if reach the max
     */
    inline void markPacked() {
        if (getPackedMarks() < MARK_MASK) info[1]++;
    }

    /**
     *  Unmark this packed node
     *  Decrease 1 the count if given flag 1, otherwise clear count
     *  Counting unchange if 0
     */
    inline void unmarkPacked(bool f) {
        if (getPackedMarks() > 0) info[1]--;
    }

    /**
     * Unpack and get the child edge's reduction rule 
     * 
     * @param child the index of the child edges: 0 ... 3
     * @return ReductionRule 
     */
    inline ReductionRule unpackEdgeRule(char child) const {
        if ((!(info[1] & NODE_TYPE_MASK) && child > 1) || child > 3) {
            // child index is out of the valid range, returns a value but throw error.
            throw error(ErrCode::INVALID_BOUND, __FILE__, __LINE__);
        }
        uint32_t EDGE_RULE_MASK = ((0x01 << 4) - 1) << (16 + 4 * (3 - child));
        return (ReductionRule)(info[1] & EDGE_RULE_MASK);
    }

    /**
     * Unpack and get the child node handle
     * 
     * @param child the index of the child node: 0 ... 3
     * @return NodeHandle 
     */
    inline NodeHandle unpackNodeHandle(char child) const {
        if ((!(info[1] & NODE_TYPE_MASK) && child > 1) || child > 3) {
            // child index is out of the valid range, returns a value but throw error.
            throw error(ErrCode::INVALID_BOUND, __FILE__, __LINE__);
        }
        return (NodeHandle)info[2 + child];
    }

    /**
     * Unpack and get the child node level
     * 
     * @param child the index of the child node: 0 ... 3
     * @return uint16_t 
     */
    inline uint16_t unpackNodeLevel(char child) const {
        if ((!(info[1] & NODE_TYPE_MASK) && child > 1) || child > 3) {
            // child index is out of the valid range, returns a value but throw error.
            throw error(ErrCode::INVALID_BOUND, __FILE__, __LINE__);
        }
        uint32_t NODE_LEVEL_MASK = ((0x01 << 16) - 1) << (16 * (1 - (child % 2)));
        return (uint16_t)((((info[1] & NODE_TYPE_MASK) ? info[6 + (child / 2)] : info[4]) 
                            & NODE_LEVEL_MASK) >> (16 * (1 - (child % 2))));
    }

    /**
     * Unpack and get the child edge's complement flag
     * 
     * @param child the index of the child edge: 0 ... 3
     * @return true 
     * @return false 
     */
    inline bool unpackEdgeComp(char child) const {
        if ((!(info[1] & NODE_TYPE_MASK) && child > 1) || child > 3) {
            // child index is out of the valid range, returns a value but throw error.
            throw error(ErrCode::INVALID_BOUND, __FILE__, __LINE__);
        }
        if (child == 0) return 0;
        return info[1] & (0x01 << (13 + (3 - child)));
    }

    /**
     * Unpack and get the child edge's swap flag
     * 
     * @param child the index of the child edge: 0 ... 3
     * @param swap  for Mxnode: 0 for from swap; 1 for to swap
     * @return true 
     * @return false 
     */
    inline bool unpackEdgeSwap(char child, bool swap) const {
        if ((!(info[1] & NODE_TYPE_MASK) && child > 1) || child > 3) {
            // child index is out of the valid range, returns a value but throw error.
            throw error(ErrCode::INVALID_BOUND, __FILE__, __LINE__);
        }
        if (!(info[1] & NODE_TYPE_MASK)) return info[1] & (0x01 << (10 + 2 * (1 - (child % 2))));
        return info[1] & (0x01 << (5 + 2 * (3 - child) + (1 - swap)));
    }

    /**
     * Unpack and get the child edge's label
     * 
     * @param child the index of the child edge: 0 ... 3
     * @return EdgeLabel 
     */
    inline EdgeLabel unpackEdgeLabel(char child) const {
        EdgeLabel label = 0;
        packRule(label, this->unpackEdgeRule(child));
        packComp(label, this->unpackEdgeComp(child));
        packSwap(label, this->unpackEdgeSwap(child, 0));
        packSwapTo(label, this->unpackEdgeSwap(child, 1));
        return label;
    }

    /**
     * Hash a packed node
     * 
     * @return uint64_t 
     */
    inline uint64_t hash() const {
        hash_stream hs;
        hs.start(0);
        bool isMx = info[1] & NODE_TYPE_MASK;
        // push info
        int n = (isMx) ? 8 : 5;
        hs.push(info[1] >> 4);
        for (int i=2; i<n; i++) hs.push(info[i]);
        if (isMx) {
            for (int i=0; i<3; i++) hs.push(values[i], (std::size_t)sizeof(uint64_t));
        } else {
            hs.push(values, (std::size_t)sizeof(uint64_t));
        }
        return (uint64_t)hs.finish64();
    }

    /**
     * Check if this equals to the given packed node
     * 
     * @param node
     * @return true 
     * @return false 
     */
    inline bool operator==(const Node& node) const {
        // labels
        uint32_t LABEL_MASK = (0x01 << 28) >> 4;
        if (((info[1] & LABEL_MASK) != (node.info[1] & LABEL_MASK))) return 0;
        // node handles and levels
        bool isMx = info[1] & NODE_TYPE_MASK;
        int n = (isMx) ? 8 : 5;
        for (int i=2 ; i<n; i++) {
            if (info[i] != node.info[i]) return 0;
        }
        // values
        if (isMx) {
            for (int i=0; i<3; i++) {
                if (values[i] != node.values[i]) return 0;
            }
        } else {
            return values == node.values;
        }
        return 1;
    }

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    /// ============================================================
    uint32_t* info;         // Next pointer, edge rules, edge flags, node handles, and levels
    uint64_t* values;       // Edge values
};


#endif