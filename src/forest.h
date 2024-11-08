#ifndef BRAVE_DD_FOREST_H
#define BRAVE_DD_FOREST_H

#include "defines.h"
#include "setting.h"
#include "edge.h"
#include "node.h"
#include "function.h"
#include "node_manager.h"
#include "unique_table.h"
#include "statistics.h"

namespace BRAVE_DD {
    class Forest;
};

// ******************************************************************
// *                                                                *
// *                                                                *
// *                          forest class                          *
// *                                                                *
// *                                                                *
// ******************************************************************
/**
 * @brief Forest class.
 * 
 * A data structure for managing collections of functions (or sets, 
 * or relations, depending on your conceptual view) represented in a 
 * single decision diagram forest over a common domain.
 *
 */
class BRAVE_DD::Forest {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    /**
     * @brief Construct a new Forest object from setting.
     * 
     * @param setting       The specification settings
     */
    Forest(const ForestSetting &setting);
    /**
     * @brief Destroy the Forest object
     */
    ~Forest();

    /// Methods =====================================================
    /************** Manipulating Terminal Node Handle ***************/
    /**
     * @brief Check if the given EdgeHandle is target to terminal.
     * Note: terminal nodes are not stored, they are represented 
     *       by a special NodeHandle value.
     * 
     * @param handle        The given EdgeHandle.
     * @return true         – If it's terminal.
     * @return false        – If not.
     */
    bool isTerminal(EdgeHandle handle);
    /**
     * @brief Get the terminal value for the given EdgeHandle.
     * Note: it would check if the given handle is terminal, 
     *       and return -1 if not.
     * 
     * @param handle        The given EdgeHandle.
     * @return EdgeValue     – Output termianl value wrapper.
     */
    EdgeValue getTerminalValue(EdgeHandle handle);
    /**
     * @brief Make the special NodeHandle for terminal Ω. 
     * This is usually used for edge valued BDDs.
     * 
     * @return NodeHandle   – Output terminal node handle.
     */
    NodeHandle makeTerminal();
    /**
     * @brief Make the special NodeHandle for termianl with 
     * a given value.
     * 
     * @param val           The given terminal value.
     * @return NodeHandle   – Output terminal node handle.
     */
    NodeHandle makeTerminal(EdgeValue val);

    /**
     * @brief Get the target node level of a given edge.
     * 
     * @param edge          The given edge.
     * @return uint16_t
     */
    inline uint16_t getNodeLevel(Edge edge) {return unpackLevel(edge.handle);};
    /**
     * @brief Get the target Node in the NodeManager for the given edge.
     * 
     * @param edge          The given edge.
     * @return Node
     */
    Node getNode(Edge edge);
    /**
     * @brief Get the unpacked target Node of the given edge.
     * 
     * @param edge          The given edge
     * @return Node         – Output a new unpacked node.
     */
    Node getNode(EdgeHandle edge);
    /**
     * @brief Get the unpacked Node from the given NodeHandle
     * 
     * @param level         The given node level.
     * @param handle        The given node handle.
     * @return Node 
     */
    inline Node& getNode(const uint16_t level, const NodeHandle handle) const{
        return nodeMan->getNodeFromHandle(level, handle);
    }

    inline NodeHandle insertNode(const uint16_t level, const Node& node) {
        return uniqueTable->insert(level, node);
    }

    /**
     * @brief Get the child Edge of the target node for a given edge with 
     * the index of child.
     * 
     * @param edge          The given edge.
     * @param index         The index of child.
     * @return Edge
     */
    Edge getChildEdge(Edge edge, int index);
    /**
     * @brief Get the child EdgeLabel for a given edge with the index of child.
     * 
     * @param handle        The given edge.
     * @param index         The index of child.
     * @return Edge
     */
    EdgeLabel getChildLabel(NodeHandle handle, int index);
    /**
     * @brief Get the next NodeHandle in the UniqueTable for a given node handle.
     * 
     * @param level         The given node level.
     * @param handle        The given node handle.
     * @return NodeHandle 
     */
    inline NodeHandle getNodeNext(const uint16_t level, const NodeHandle handle) const {
        return nodeMan->getNodeFromHandle(level, handle).getNext();
    }
    /**
     * @brief Set the next NodeHandle in the UniqueTable for a given node handle.
     * 
     * @param level         The givien node level.
     * @param handle        The givien node handle.
     * @param next          The next handle to set.
     */
    inline void setNodeNext(const uint16_t level, const NodeHandle handle, const NodeHandle next) {
        nodeMan->getNodeFromHandle(level, handle).setNext(next);
    }
    /**
     * @brief Get the Node Hash
     * 
     * @param level         The given node level.
     * @param handle        The given node handle.
     * @return uint64_t     The hash value.
     */
    inline uint64_t getNodeHash(const uint16_t level, const NodeHandle handle) const {
        return nodeMan->getNodeFromHandle(level, handle).hash();
    }
    /**
     * @brief Check if the given unpacked node and the node handle are duplicates 
     * (i.e., having the same child labels and target nodes).
     * 
     * @param level         The given node level.
     * @param handle        The given node handle.
     * @param node          The given unpacked node to compare.
     * @return true 
     * @return false 
     */
    bool areDuplicates(uint16_t level, NodeHandle handle, Node node);

    /************************* Reduction ****************************/
    /**
     * @brief Normalize the given unpacked node "P". EdgeLabel "*out" specifies 
     * the incoming edge rule/value and flags, which may be changed by the normalization.
     * 
     * @param lvl           The given node level.
     * @param P             The given unpacked node waiting for normalization.
     * @param out           Output: edge label (rule/value, flags).
     */
    void normalizeNode(uint16_t lvl, Node& P, EdgeLabel* out);
    /**
     * @brief Reduce the given unpacked node "P" by checking the forbidden patterns 
     * of nodes. Edge "*out" will be written the long edge that represent unpacked 
     * node "*P".
     * 
     * @param lvl           The given node level.
     * @param P             The given unpacked node waiting for reduction.
     * @param out           Output: reduced edge (label, target node handle).
     */
    void reduceNode(uint16_t lvl, Node& P, Edge* out);
    /**
     * @brief Merge the incoming edge with EdgeLabel "label", which is respect of 
     * level "lvl1", target to the node at level "lvl2"; edge "*reduced" represents 
     * the node at level "lvl2" after calling reduceNode method. Edge "*out" can not 
     * be null, it will be written the merged edge.
     * 
     * @param lvl1          The represent level of the incoming edge.
     * @param lvl2          The target node level of the incoming edge.
     * @param label         The incoming edge label.
     * @param reduced       The reduced edge.
     * @param out           Output: merged edge (label, target node handle).
     */
    void mergeEdge(uint16_t lvl1, uint16_t lvl2, EdgeLabel label, Edge* reduced, Edge* out);
    /**
     * @brief Reduce the incoming edge with label "label", which is respect of level 
     * "lvl", target to the unpacked node "P"; Edge "*out" can not be null, it will 
     * be written the reduced edge.
     * 
     * @param lvl           The represent level of the incoming edge.
     * @param label         The incoming edge label.
     * @param P             The given target unpacked node of the incoming edge.
     * @param out           Output: reduced edge (label, target node handle).
     */
    void reduceEdge(uint16_t lvl, EdgeLabel label, uint16_t nodeLvl, std::vector<Edge> child, Edge* out);

    inline NodeManager* getNodeMan() {return nodeMan;}


    /************************* Within Operations ********************/
    

    /***************************** Cardinality **********************/
    uint64_t countNodes();   // all Funcs
    uint64_t countNodes(FuncArray funcs);
    uint64_t countNodesAtLevel(uint16_t lvl);
    uint64_t countNodesAtLevel(uint16_t lvl, Func func);
    uint64_t countNodesAtLevel(uint16_t lvl, FuncArray funcs);

    uint64_t mass(Func func);

    uint64_t count(Func func, int val); // ...
    // mpz_t count(); // gmp here?
    uint64_t count(Func func, long min, long max); // min and max are both included
    uint64_t count(Func func, double min, double max); // min and max are both included
    uint64_t count(Func func, SpecialValue val);
    unsigned minValue(Func func); // return type
    unsigned maxValue(Func func); // return type
    // TBD

    /************************* Garbage Collection *******************/
    void deleteNode(NodeHandle handle);
    void markSweep();
    // TBD

    /************************* Statistics Information ***************/
    uint64_t getPeakNodes();    // largest result of getCurrentNodes(), since the last call to resetPeakNodes()
    uint64_t getCurrentNodes(); // number of nodes in UT, including disconnected
    void resetPeakNodes();
    // TBD

    /****************************** I/O *****************************/
    // for transform files
    void exportFunc(std::ostream& out, FuncArray func);
    void exportForest(std::ostream& out);
    FuncArray importFunc(std::istream& in);
    void importForest(std::istream& in);
    
    // TBD


    /************************* Setting Information ******************/
    /**
     * @brief Get the ForestSetting used by this forest.
     */
    inline const ForestSetting& getSetting() const {return setting;}

    /*************************** Reordering *************************/
    void shiftUp(unsigned lvl);
    void shiftDown(unsigned lvl);
    void reorder(unsigned* level2Var);

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    /// Helper Methods ==============================================
    /* Marker */
    /**
     * @brief Unmark all nodes in the forest. This is usually used to initialize 
     * for counting the marked nodes or sweeping the unmarked nodes.
     * 
     */
    void unmark();
    /**
     * @brief Mark all the nonterminal nodes reachable from the given Func edge
     * in the forest.
     * 
     * @param Func          The Func edge.
     */
    void markNodes(Func Func);
    void markNodes(Edge edge);
    /**
     * @brief Mark all the nonterminal nodes reachable from all Func nodes in 
     * the forest.
     * 
     */
    void markAllFuncs();


    /// =============================================================
        ForestSetting       setting;        // Specification setting of this forest.
        NodeManager*        nodeMan;        // Node manager.
        UniqueTable*        uniqueTable;    // Unique table.
        Func*               funcs;          // Registry of Func edges.
        FuncArray*          funcSets;       // Sets of Func used for I/O.
        Statistics*         stats;          // Performance measurement.
        // int                 numInfo;        // The number of slots required in Node

};


#endif