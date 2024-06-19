#ifndef BRAVE_DD_EDGE_H
#define BRAVE_DD_EDGE_H

#include "defines.h"
#include "edge_label.h"

#include <string>

namespace BRAVE_DD {
    class Edge;
    class Forest;
    
    // file I/O
    // TBD
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
        /// Construct and attach to a forest.
        Edge(Forest* parent);

        // / Copy Constructor.
        Edge(const Edge &e);

        /// Destructor. Will notify parent as appropriate.
        ~Edge();

        //******************************************
        //  Getters
        //******************************************
        inline Forest* getForest() const {return parentForest;};

        inline bool isAttachedTo(const Forest* p) const {
            return getForest() == p;
        }
        inline bool isSameForest(const Edge &e) const {
            return parentForest == e.getForest();
        }
        inline NodeHandle getTarget() const {
            return targetNode;
        }
        inline const EdgeLabel& getLabel() const {
            return label;
        }
        inline ValueType getEdgeValueType() const {
            return label.getValueType();
        }
        template<typename T>
        inline void getEdgeValueTo(T &v) const {
            label.getValueTo(v);
        }
        inline ReductionRule getEdgeRule() const {
            return label.getRule();
        }
        inline bool getEdgeComp() const {
            return label.getComp();
        }
        inline bool getEdgeSwap() const {
            return label.getSwap();
        }
        //
        // More getter TBD here
        //

        //******************************************
        //  Setters
        //******************************************
        /// Attach to a forest.
        void attach(Forest* p);

        /// Detach from the forest.
        inline void detach() { attach(nullptr); }

        inline void setTarget(NodeHandle t) {
            targetNode = t;
        }
        inline void setEdgeValueType(ValueType t) {
            label.setValueType(t);
        }
        template<typename T>
        inline void setEdgeValue(T v) {
            label.setValue(v);
        }
        inline void setEdgeRule(ReductionRule r) {
            label.setRule(r);
        }
        inline void setEdgeComp(bool c) {
            label.setComp(c);
        }
        inline void setEdgeSwap(bool s) {
            label.setSwap(s);
        }
        template<typename T>
        inline void set(NodeHandle handle, T val) {
            setTarget(handle);
            setEdgeValue(val);
        }
        inline void set(NodeHandle handle, ReductionRule rul) {
            setTarget(handle);
            setEdgeRule(rul);
        }

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
        friend class Forest;
        
        //
        // Actual edge information
        //
        NodeHandle      targetNode;     // Target node
        EdgeLabel       label;          // Label including rule, value, flags

        Forest*         parentForest;   // parent forest

        std::string     display;        // for displaying if needed in the future

        //
        // for the dd_edge registry in the parent forest
        //
        /// Previous edge in forest registry
        Edge* prevEdge;

        /// Next edge in forest registry
        Edge* nextEdge;
};


#endif