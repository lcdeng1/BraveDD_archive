#ifndef BRAVE_DD_NODE_H
#define BRAVE_DD_NODE_H

#include "defines.h"
#include "edge.h"

namespace BRAVE_DD {
    class Node;
    class Mxde;

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
    ~Node();

    /// Methods =====================================================
    /**
     *  Get the down node handle for the given index of child
     *  0: low/left
     *  1: high/right
     */
    inline Edge getChild(bool i) const {
        return i?child1:child0;
    }
    
    
    /// Get this node's hash value
    inline unsigned long hash() const { return hashValue; }

    /// Compute the node's hash value
    void computeHash();

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    Edge child0, child1;
    unsigned long hashValue;    // Hash of the node

};

// ******************************************************************
// *                                                                *
// *                           Node  class                          *
// *                                                                *
// ******************************************************************

class BRAVE_DD::Mxde {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    Mxde();
    ~Mxde();

    /// Get this node's hash value
    inline unsigned long hash() const { return hashValue; }

    /// Compute the node's hash value
    void computeHash();

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    Edge child00, child01, child10, child11;
    unsigned long hashValue;
};

#endif