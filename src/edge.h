#ifndef BRAVE_DD_EDGE_H
#define BRAVE_DD_EDGE_H

#include "defines.h"
#include "setting.h"

namespace BRAVE_DD {
    enum class SpecialValue {
        POS_INF,
        NEG_INF,
        UNDEF
    };
    /**
     *  Labels for edge rule and flags storage
     *  Each label is constructed as:
     *            [ unused(1 bit) | rule(4 bits) | flags(3 bits)]
     * 
     *  flags(3 bits): swap(_from) | swap_to | complement
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
    static inline bool unpackSwap(const EdgeLabel label)
    {
        uint64_t SWAP_MASK = (uint64_t)(0x01 << 2);
        return label & SWAP_MASK;
    }
    static inline bool unpackSwapTo(const EdgeLabel label)
    {
        uint64_t SWAP_MASK = (uint64_t)(0x01 << 1);
        return label & SWAP_MASK;
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
        uint64_t COMP_MASK = (uint64_t)(0x01) << 48;
        return handle & COMP_MASK;
    }
    /* Get the swap flag from the given EdgeHandle */
    static inline bool unpackSwap(const EdgeHandle handle)
    {
        uint64_t SWAP_MASK = (uint64_t)(0x01) << 50;
        return handle & SWAP_MASK;
    }
    static inline bool unpackSwapTo(const EdgeHandle handle)
    {
        uint64_t SWAP_MASK = (uint64_t)(0x01) << 49;
        return handle & SWAP_MASK;
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
    /* Packing */
    static inline void packRule(EdgeHandle handle, ReductionRule rule)
    {
        handle = handle | ((uint64_t)rule << 51);
    }
    static inline void packLevel(EdgeHandle handle, uint16_t level)
    {
        handle = handle | ((uint64_t)level << 32);
    }
    static inline void packComp(EdgeHandle handle, bool comp)
    {
        handle = handle | ((uint64_t)comp << 48);
    }
    static inline void packSwap(EdgeHandle handle, bool swap)
    {
        handle = handle | ((uint64_t)swap << 50);
    }
    static inline void packSwapTo(EdgeHandle handle, bool swap)
    {
        handle = handle | ((uint64_t)swap << 49);
    }
    static inline void packTarget(EdgeHandle handle, NodeHandle target)
    {
        handle = handle | ((uint64_t)target);
    }


    class EdgeValue;
    class Edge;
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
    EdgeValue(long l);
    EdgeValue(double d);
    EdgeValue(float f);

    ValueType getType() const { return valueType; }
    inline void getValueTo(void* p, ValueType type) const {
        switch (type) {
            case VOID:
                *((SpecialValue*) p) = getSpecialValue();
                return;
            case INT:
                *((int*) p) = getIntValue();
                return;
            case LONG:
                *((long*) p) = getLongValue();
                return;
            case FLOAT:
                *((float*) p) = getFloatValue();
                return;
            case DOUBLE:
                *((double*) p) = getDoubleValue();
                return;
            default:
                throw error(BRAVE_DD::ErrCode::MISCELLANEOUS, __FILE__, __LINE__);
        }
    }
    inline void setValue(const void* p, ValueType type) {
        switch (type) {
            case VOID:
                setSpecial(p);
            case INT:
                setInt(p);
            case LONG:
                setLong(p);
            case FLOAT:
                setFloat(p);
            case DOUBLE:
                setDouble(p);
            default:
                throw error(BRAVE_DD::ErrCode::MISCELLANEOUS, __FILE__, __LINE__);
            }
    }
    inline EdgeValue& operator=(const EdgeValue& val) {
        if (equals(val)) return *this;
        init(val);
        return *this;
    }
    inline bool operator==(const EdgeValue& val) const {
        return equals(val);
    }
    inline bool operator!=(const EdgeValue& val) const {
        return !equals(val);
    }
    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    inline int getIntValue() const { return intValue;}
    inline long getLongValue() const { return longValue;}
    inline float getFloatValue() const { return floatValue;}
    inline double getDoubleValue() const { return doubleValue;}
    inline SpecialValue getSpecialValue() const { return special;}
    // inline void setVoid() {valueType = VOID;}
    inline void setInt(const void *p) {
        BRAVE_DD_DCASSERT(p);
        valueType = INT;
        intValue = *((const int*) p);
    }
    inline void setLong(const void *p) {
        BRAVE_DD_DCASSERT(p);
        valueType = LONG;
        longValue = *((const long*) p);
    }
    inline void setFloat(const void *p) {
        BRAVE_DD_DCASSERT(p);
        valueType = FLOAT;
        floatValue = *((const float*) p);
    }
    inline void setDouble(const void *p) {
        BRAVE_DD_DCASSERT(p);
        valueType = DOUBLE;
        doubleValue = *((const double*) p);
    }
    inline void setSpecial(const void *p) {
        BRAVE_DD_DCASSERT(p);
        valueType = VOID;
        special = *((const SpecialValue*) p);
    }
    inline bool equals(const EdgeValue& val) const {
        bool isEqual = 1;
        if (valueType != val.valueType) return !isEqual;
        switch (valueType) {
            case VOID:
                return isEqual = special == val.special;
            case INT:
                return isEqual = intValue == val.intValue;
            case LONG:
                return isEqual = longValue == val.longValue;
            case FLOAT:
                return isEqual = floatValue == val.floatValue;      // Precision? TBD
            case DOUBLE:
                return isEqual = doubleValue == val.doubleValue;    // Precision? TBD
            default:
                throw error(BRAVE_DD::ErrCode::MISCELLANEOUS, __FILE__, __LINE__);
        }
        return isEqual;
    }
    inline void init(const EdgeValue& val) {
        valueType = val.valueType;
        switch (valueType) {
            case VOID:
                special = val.special;
            case INT:
                intValue = val.intValue;
            case LONG:
                longValue = val.longValue;
            case FLOAT:
                floatValue = val.floatValue;
            case DOUBLE:
                doubleValue = val.doubleValue;
            default:
                throw error(BRAVE_DD::ErrCode::MISCELLANEOUS, __FILE__, __LINE__);
        }
    }

    /*-------------------------------------------------------------*/
    ValueType valueType;
    /// Values
    union {
        int             intValue;
        long            longValue;
        float           floatValue;
        double          doubleValue;
    };
    SpecialValue special;
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
        Edge(const EdgeHandle h, EdgeValue val);
        /// Destructor.
        ~Edge();

        inline Edge& operator=(const Edge& e) {
            if (equals(e)) return *this;
            init(e);
            return *this;
        }

        inline bool operator==(const Edge& e) const {
            return equals(e);
        }
        inline bool operator!=(const Edge& e) const {
            return !equals(e);
        }
    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
        inline void init(const Edge& e) {
            handle = e.handle;
            value = e.value;
        }
        inline bool equals(const Edge e) const {
            return (handle == e.handle) && (value == e.value);
        }
        /* Getters and Setters under Forest */
        friend class Forest;
        friend class Func;
        friend class Node;
        friend class Mxnode;
        friend class PackedNode;

        /* Actual edge information */
        EdgeHandle      handle;     // Rule, flags, taget node and level.
        EdgeValue       value;      // Edge value.

        // std::string     display;    // for displaying if needed in the future
};

#endif