#ifndef BRAVE_DD_NODE_H
#define BRAVE_DD_NODE_H

#include "defines.h"
#include "edge.h"

namespace BRAVE_DD {
    class Node;
    class Mxnode;

}

// ******************************************************************
// *                                                                *
// *                           Node  class                          *
// *                                                                *
// ******************************************************************

class BRAVE_DD::Node {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    Node();
    Node(uint16_t lvl);
    ~Node();

    /// Methods =====================================================
    /**
     *  Get the down node handle for the given index of child
     *  
     * @param i 0: low/left; 1: high/right
     */
    inline Edge getChild(bool i) const {
        return i?child1:child0;
    }
    /**
     * Set the child edge from the given edge
     * 
     * @param i 0: low/left; 1: high/right
     * @param e the given edge
     */
    inline void setChild(bool i, Edge& e) {
        if (i == 0) {
            child0 = e;
        } else if (i == 1) {
            child1 = e;
        } else {
            throw error(ErrCode::INVALID_BOUND, __FILE__, __LINE__);
        }
    }
    
    /// Get this node's hash value
    inline unsigned long hash() const { return hashValue; }

    /// Compute the node's hash value
    void computeHash();

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    friend class PackedNode;
    friend class Forest;
    uint16_t level;
    Edge child0, child1;
    unsigned long hashValue;    // Hash of the node

};

// ******************************************************************
// *                                                                *
// *                           Mxnode  class                          *
// *                                                                *
// ******************************************************************

class BRAVE_DD::Mxnode {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    Mxnode();
    Mxnode(uint16_t lvl);
    ~Mxnode();

    /// Methods =====================================================
    /**
     *  Get the down node handle for the given index of child
     *  
     * @param i 0: low/left; 1: high/right
     */
    inline Edge getChild(int i) const {
        switch (i)
        {
        case 0:
            return child00;
        case 1:
            return child01;
        case 2:
            return child10;
        case 3:
            return child11;
        default:
            throw error(ErrCode::INVALID_BOUND, __FILE__, __LINE__);
        }
    }
    /**
     * Set the child edge from the given edge
     * 
     * @param i 0: low/left; 1: high/right
     * @param e the given edge
     */
    inline void setChild(int i, Edge& e) {
        switch (i)
        {
        case 0:
            child00 = e;
        case 1:
            child01 = e;
        case 2:
            child10 = e;
        case 3:
            child11 = e;
        default:
            throw error(ErrCode::INVALID_BOUND, __FILE__, __LINE__);
        }
    }


    /// Get this node's hash value
    inline unsigned long hash() const { return hashValue; }

    /// Compute the node's hash value
    void computeHash();

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    uint16_t level;
    Edge child00, child01, child10, child11;
    unsigned long hashValue;
};

#endif