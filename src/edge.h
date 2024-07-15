#ifndef BRAVE_DD_EDGE_H
#define BRAVE_DD_EDGE_H

#include "defines.h"
#include "setting.h"

namespace BRAVE_DD {
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

        
        
        // inline NodeHandle getTarget() const {
        //     return targetNode;
        // }
        // inline const EdgeLabel& getLabel() const {
        //     return label;
        // }
        // inline ValueType getEdgeValueType() const {
        //     return label.getValueType();
        // }
        // template<typename T>
        // inline void getEdgeValueTo(T &v) const {
        //     label.getValueTo(v);
        // }
        // inline ReductionRule getEdgeRule() const {
        //     return label.getRule();
        // }
        // inline bool getEdgeComp() const {
        //     return label.getComp();
        // }
        // inline bool getEdgeSwap() const {
        //     return label.getSwap();
        // }
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
    Forest* parent;     // parent forest
    /* For the roots registry in the parent forest */
    /// Previous root edge in parent forest
    Root* prevRoot;
    /// Next root edge in parent forest
    Root* nextRoot;
};

#endif