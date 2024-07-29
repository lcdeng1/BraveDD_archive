#ifndef BRAVE_DD_EDGE_H
#define BRAVE_DD_EDGE_H

#include "defines.h"
#include "setting.h"

namespace BRAVE_DD {
    /**
     *  Labels for edge rule and flags storage
     *  Each label is constructed as:
     *            [ unused(1 bit) | rule(4 bits) | flags(3 bits)]
     *  
     */
    typedef uint8_t EdgeLabel;

    /* Methods of EdgeLabel */

    /* Get the reduction rule from the given EdgeLabel */
    static inline ReductionRule unpackRule(const EdgeLabel label)
    {
        uint8_t RULE_MASK = (uint8_t)((0x01<<4) - 1) << 3;
        return (ReductionRule)((label & RULE_MASK) >> 3);
    }
    /* Get the complement flag from the given EdgeLabel */
    static inline bool unpackComp(const EdgeLabel label)
    {
        uint8_t COMP_MASK = (uint8_t)0x01;
        return label & COMP_MASK;
    }
    /* Get the swap flag from the given EdgeLabel */
    static inline bool unpackSwap(const EdgeLabel label, const bool isTo)
    {
        uint64_t SWAP_MASK = (uint64_t)(0x01 << 1);
        return isTo ? (label & SWAP_MASK) : (label & (SWAP_MASK << 1));
    }
    /* Packing */
    static inline void packRule(EdgeLabel label, ReductionRule rule)
    {
        label = label | ((uint8_t)(rule) << 3);
    }
    static inline void packComp(EdgeLabel label, bool comp)
    {
        label = label | ((uint8_t)comp);
    }
    static inline void packSwap(EdgeLabel label, bool swap)
    {
        label = label | ((uint8_t)swap << 2);
    }
    static inline void packSwapTo(EdgeLabel label, bool swap)
    {
        label = label | ((uint8_t)swap << 1);
    }

    /**
     *  Handles for edges storage
     *  This effectively limits the number of possible nodes per forest.
     *  Each handle is constructed as:
     *            [ unused(9 bits) | rule(4 bits) | flags(3 bits) | level(16 bits) | nodeIdx(32 bits) ]
     *  "nodeIdx" limits the number of nodes per level in node manager.
     *  "flags": swap(_from) swap_to complement
     * 
     *  [TBD] v
     *  The number of bits occupied by each part depends on the forest setting:
     *      Bits of "rule" = |log(numRules)|;
     *      Bits of "flags" = complement bit + swap bits;
     *      Bits of "level" = |log(maxLevel)|;
     *      Bits of "nodeIdx" = the remain bits;
     */
    typedef uint64_t EdgeHandle;
    
    /* Methods of EdgeHandle */

    /* Get the reduction rule from the given EdgeHandle */
    static inline ReductionRule unpackRule(const EdgeHandle handle)
    {
        uint64_t RULE_MASK = (uint64_t)((0x01<<4) - 1) << 51;
        return (ReductionRule)((handle & RULE_MASK) >> 51);
    }
    /* Get the level of the target node from the given EdgeHandle */
    static inline uint16_t unpackLevel(const EdgeHandle handle)
    {
        uint64_t LEVEL_MASK = (uint64_t)((0x01<<16) - 1) << 32;
        return (uint16_t)((handle & LEVEL_MASK) >> 32);
    }
    /* Get the complement flag from the given EdgeHandle */
    static inline bool unpackComp(const EdgeHandle handle)
    {
        uint64_t COMP_MASK = (uint64_t)(0x01) << 51;
        return handle & COMP_MASK;
    }
    /* Get the swap flag from the given EdgeHandle */
    static inline bool unpackSwap(const EdgeHandle handle, const bool isTo)
    {
        uint64_t SWAP_MASK = (uint64_t)(0x01) << 52;
        return isTo ? (handle & SWAP_MASK) : (handle & (SWAP_MASK << 1));
    }
    /* Get the target node index from the given EdgeHandle */
    static inline NodeHandle unpackNode(const EdgeHandle handle)
    {
        uint64_t NODE_MASK = ((uint64_t)(0x01)<<32) - 1;
        return (NodeHandle)(handle & NODE_MASK);
    }
    static inline EdgeLabel unpackLabel(const EdgeHandle handle)
    {
        uint64_t LABEL_MASK = (uint64_t)((0x01<<8) - 1) << 48;
        return (EdgeLabel)((handle & LABEL_MASK) >> 48);
    }

    class EdgeValue;
    class Edge;
    class Root;
    class Forest;
    // file I/O
    // TBD
};


// ******************************************************************
// *                                                                *
// *                                                                *
// *                      EdgeValue class                           *
// *                                                                *
// *                                                                *
// ******************************************************************
class BRAVE_DD::EdgeValue {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    EdgeValue();
    EdgeValue(int i);
    EdgeValue(double d);
    EdgeValue(float f);

    ValueType getType() const { return valueType; }
    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    ValueType valueType;
    /// Values
    union {
        int             intValue;
        long            longValue;
        float           floatValue;
        double          doubleValue;
    };
};
// ******************************************************************
// *                                                                *
// *                                                                *
// *                          Edge class                            *
// *                                                                *
// *                                                                *
// ******************************************************************
class BRAVE_DD::Edge {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
        Edge();
        // / Copy Constructor.
        Edge(const Edge &e);

        /// Destructor.
        ~Edge();

        //******************************************
        //  Getters
        //******************************************

        
        
        inline NodeHandle getTarget() const {
            return unpackNode(handle);
        }
        inline EdgeLabel getLabel() const {
            return unpackLabel(handle);
        }
        inline ValueType getEdgeValueType() const {
            return value.getType();
        }
        // template<typename T>
        // inline void getEdgeValueTo(T &v) const {
        //     label.getValueTo(v);
        // }
        inline ReductionRule getEdgeRule() const {
            return unpackRule(handle);
        }
        inline bool getEdgeComp() const {
            return unpackComp(handle);
        }
        inline bool getEdgeSwap(bool isTo) const {
            return unpackSwap(handle, isTo);
        }
        //
        // More getter TBD here
        //

        //******************************************
        //  Setters
        //******************************************
        

        

        // inline void setTarget(NodeHandle t) {
        //     targetNode = t;
        // }
        // inline void setEdgeValueType(ValueType t) {
        //     label.setValueType(t);
        // }
        // template<typename T>
        // inline void setEdgeValue(T v) {
        //     label.setValue(v);
        // }
        // inline void setEdgeRule(ReductionRule r) {
        //     label.setRule(r);
        // }
        // inline void setEdgeComp(bool c) {
        //     label.setComp(c);
        // }
        // inline void setEdgeSwap(bool s) {
        //     label.setSwap(s);
        // }
        // template<typename T>
        // inline void set(NodeHandle handle, T val) {
        //     setTarget(handle);
        //     setEdgeValue(val);
        // }
        // inline void set(NodeHandle handle, ReductionRule rul) {
        //     setTarget(handle);
        //     setEdgeRule(rul);
        // }

        //
        // More setter TBD here
        //
        /// Complement edge rule
        void compEdgeRule();
        /// Swap edge rule
        void swapEdgeRule();

        //******************************************
        //  Check for equality
        //******************************************
        bool equals(const Edge& e);
        bool isSameForest(const Edge& e);
    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
        /* Getters and Setters under Forest */
        friend class Forest;

        /* Actual edge information */
        EdgeHandle      handle;     // Rule, flags, taget node and level.
        EdgeValue       value;      // Edge value.

        std::string     display;    // for displaying if needed in the future
};

// ******************************************************************
// *                                                                *
// *                                                                *
// *                          Root class                            *
// *                                                                *
// *                                                                *
// ******************************************************************
class BRAVE_DD::Root {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    Root();
    Root(Forest* f);
    Root(Forest* f, const Edge& e);
    ~Root();

    inline Forest* getForest() const {return parent;};
    inline bool isAttachedTo(const Forest* p) const {return getForest() == p;}
    inline bool isSameForest(const Root &e) const {return parent == e.getForest();}
    
    /// Attach to a forest.
    void attach(Forest* p);
    /// Detach from the forest.
    inline void detach() { attach(nullptr); }

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    Forest*     parent;     // parent forest
    Edge        edge;       // edge information
    /* For the roots registry in the parent forest */
    Root*       prevRoot;   // Previous root edge in parent forest
    Root*       nextRoot;   // Next root edge in parent forest
};

#endif