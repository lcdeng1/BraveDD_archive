#ifndef BRAVE_DD_FOREST_H
#define BRAVE_DD_FOREST_H

#include "defines.h"
#include "setting.h"
#include "node.h"
#include "packed_node.h"
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
     * @brief Check if the given NodeHandle is terminal.
     * Note: terminal nodes are not stored, they are represented 
     *       by a special NodeHandle value.
     * 
     * @param handle        The given NodeHandle.
     * @return true         – If it's terminal.
     * @return false        – If not.
     */
    bool isTerminal(NodeHandle handle);
    /**
     * @brief Get the terminal value for the given NodeHandle.
     * Note: it would check if the given handle is terminal, 
     *       and return -1 if not.
     * 
     * @param handle        The given NodeHandle.
     * @return uint64_t     – Output termianl value.
     */
    uint64_t getTerminalValue(NodeHandle handle);
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
    NodeHandle makeTerminal(int val);
    /**
     * @brief Make the special NodeHandle for termianl with 
     * a given value.
     * 
     * @param val           The given terminal value.
     * @return NodeHandle   – Output terminal node handle.
     */
    NodeHandle makeTerminal(long val);
    /**
     * @brief Make the special NodeHandle for termianl with 
     * a given value.
     * 
     * @param val           The given terminal value.
     * @return NodeHandle   – Output terminal node handle.
     */
    NodeHandle makeTerminal(float val);
    /**
     * @brief Make the special NodeHandle for termianl with 
     * a given value.
     * 
     * @param val           The given terminal value.
     * @return NodeHandle   – Output terminal node handle.
     */
    NodeHandle makeTerminal(double val);

    /*************** Node Information from Handles ******************/
    /**
     * @brief Get the level of a given node handle.
     * 
     * @param handle        The given node handle.
     * @return unsigned 
     */
    unsigned getNodeLevel(NodeHandle handle);
    /**
     * @brief Get the PackedNode in the NodeManager for a given node handle.
     * 
     * @param handle        The given node handle.
     * @return PackedNode* 
     */
    PackedNode* getPackedNode(NodeHandle handle);
    /**
     * @brief Copy packed node information into the given unpacked node.
     * 
     * @param handle        The handle of node to retrieve.
     * @param node          The given unpacked node to copy into.
     */
    void unpackNode(NodeHandle handle, Node* node);
    /**
     * @brief Copy packed node information into a new unpacked node.
     * 
     * @param handle        The handle of node to retrieve.
     * @return Node*        – Output a new unpacked node.
     */
    Node* unpackNode(NodeHandle handle);
    /**
     * @brief Get the child NodeHandle for a given parent node handle with 
     * the index of child.
     * E.g., getChildHandle(24, 0) returns the low child NodeHandle for node 24.
     * 
     * @param handle        The given parent node handle.
     * @param index         The index of child.
     * @return NodeHandle 
     */
    NodeHandle getChildHandle(NodeHandle handle, int index);
    /**
     * @brief Get the child EdgeLabel for a given parent node handle with 
     * the index of child.
     * E.g., getChildLabel(24, 1, label) outputs the high child EdgeLabel into "label".
     * 
     * @param handle        The given parent node handle.
     * @param index         The index of child
     * @param label         Output: edge label (rule/value, flags)
     */
    void getChildLabel(NodeHandle handle, int index, EdgeLabel &label);
    /**
     * @brief Get the next NodeHandle in the UniqueTable for a given node handle.
     * 
     * @param handle        The given node handle.
     * @return NodeHandle 
     */
    NodeHandle getNodeNext(NodeHandle handle);
    /**
     * @brief Set the next NodeHandle in the UniqueTable for a given node handle.
     * 
     * @param handle        The givien node handle.
     * @param next          The next handle to set.
     */
    void setNodeNext(NodeHandle handle, NodeHandle next);
    /**
     * @brief Check if the given unpacked node and the node handle are duplicates 
     * (i.e., having the same child labels and target nodes).
     * 
     * @param handle        The given node handle.
     * @param node          The given unpacked node to compare.
     * @return true 
     * @return false 
     */
    bool areDuplicates(NodeHandle handle, Node node);

    /************************* Reduction ****************************/
    /**
     * @brief Normalize the given unpacked node "*P". EdgeLabel "*out" specifies 
     * the incoming edge rule/value and flags, which may be changed by the normalization.
     * 
     * @param P             The given unpacked node waiting for normalization.
     * @param out           Output: edge label (rule/value, flags).
     */
    void normalizeNode(Node* P, EdgeLabel* out);
    /**
     * @brief Reduce the given unpacked node "*P" by checking the forbidden patterns 
     * of nodes. Edge "*out" will be written the long edge that represent unpacked 
     * node "*P".
     * 
     * @param P             The given unpacked node waiting for reduction.
     * @param out           Output: reduced edge (label, target node handle).
     */
    void reduceNode(Node* P, Edge* out);
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
    void mergeEdge(unsigned lvl1, unsigned lvl2, EdgeLabel label, Edge* reduced, Edge* out);
    /**
     * @brief Reduce the incoming edge with label "label", which is respect of level 
     * "lvl", target to the unpacked node "*P"; Edge "*out" can not be null, it will 
     * be written the reduced edge.
     * 
     * @param lvl           The represent level of the incoming edge.
     * @param label         The incoming edge label.
     * @param P             The given target unpacked node of the incoming edge.
     * @param out           Output: reduced edge (label, target node handle).
     */
    void reduceEdge(unsigned lvl, EdgeLabel label, Node* P, Edge* out);

    /**************************** Make edge *************************/
    void makeConstant(long val, unsigned lvl, Edge* out);
    void makeVariable(unsigned lvl, Edge* out);

    /************************* Within Operations ********************/
    unsigned evaluate(unsigned lvl, Edge* root, bool* vars);
    bool evaluate(unsigned lvl, Edge* root, bool* froms, bool* tos);
    // TBD

    /***************************** Cardinality **********************/
    uint64_t countNumNodes();
    uint64_t countNumNodes(NodeHandle root);
    uint64_t countNumNodesByLevel(unsigned lvl);
    unsigned highest(Edge* e);
    unsigned lowest(Edge* e);
    // TBD

    /************************** Computing Table *********************/
    // TBD

    /************************* Garbage Collection *******************/
    void deleteNode(NodeHandle handle);
    void markSweep();
    // TBD

    /************************* Statistics Information ***************/
    uint64_t getPeakNodes();    
    // TBD

    /****************************** I/O *****************************/
    void exportEdge();
    void exportForest();
    // TBD


    /************************* Setting Information ******************/
    /**
     * @brief Get the ForestSetting used by this forest.
     */
    inline const ForestSetting& getSetting() const {return setting;}
    /**
     * @brief Check if this forest is representing relation.
     */
    inline bool isRelation() const {return setting.isRelation();}
    /**
     * @brief Get the number of levels in forest. 
     */
    inline unsigned getNumLevel() const {return setting.getNumVars();}
    /**
     * @brief Get the level index for a given variable index.
     * 
     * @param var               The variable index.
     */
    inline unsigned getLevelIndex(unsigned var) const {return setting.getLevel(var);}
    /**
     * @brief Get the variable index for a given level index.
     * 
     * @param lvl               The level index.
     */
    inline unsigned getVarIndex(unsigned lvl) const {return setting.getVar(lvl);}
    /**
     * @brief Get the range type in the forest.
     */
    inline RangeType getRangeType() const {return setting.getRangeType();}
    /**
     * @brief Get the value type in the forest.
     */
    inline ValueType getValType() const {return setting.getValType();}
    /**
     * @brief Get the max range that has been set for the forest.
     */
    inline unsigned getMaxRange() const {return setting.getMaxRange();}
    /**
     * @brief Check if the forest has set the negative infinity in range.
     */
    inline bool hasNegInf() const {return setting.hasNegInf();}
    /**
     * @brief Check if the forest has set the positive infinity in range.
     */
    inline bool hasPosInf() const {return setting.hasPosInf();}
    /**
     * @brief Check if the forest has set the undefine value in range.
     */
    inline bool hasUnDef() const {return setting.hasUnDef();}
    /**
     * @brief Get the encode mechanism for the forest.
     */
    inline EncodeMechanism getEncodeMechanism() const {return setting.getEncodeMechanism();}
    /**
     * @brief Get the reduction type for the forest.
     */
    inline ReductionType getReductionType() const {return setting.getReductionType();}
    /**
     * @brief Get the size of the applied reduction rules set for the forest.
     */
    inline int getReductionSize() const {return setting.getReductionSize();}
    /**
     * @brief Check if the forest has applied the given ReductionRule.
     */
    inline bool hasReductionRule(ReductionRule rule) const {return setting.hasReductionRule(rule);}
    /**
     * @brief Get the swap flag type that has been set for the forest.
     */
    inline SwapSet getSwapType() const {return setting.getSwapType();}
    /**
     * @brief Get the complement flag type that has been set for the forest.
     */
    inline CompSet getCompType() const {return setting.getCompType();}
    /**
     * @brief Check if the complement flag has been set for the forest.
     */
    inline bool isCompSet() const {return getCompType() == COMP;}
    /**
     * @brief Get the merge type that has been set for the forest.
     */
    inline MergeType getMergeType() const {return setting.getMergeType();}

    inline std::string getForestName() const {return setting.getName();}

    /* Manipulating Setting */
    /* [Warning]: 
            Once this forest is constructed with a given setting and after some BDDs are built,
            the setting should not be changed at will. Otherwise, it may cause unpredictable results.
    */

    /*************************** Reordering *************************/
    void shiftUp(unsigned lvl);
    void shiftDown(unsigned lvl);
    void reorder(unsigned* level2Var);

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    /****************************** Edges ***************************/
    /// Getters and setters for edges (maybe move to the public? TBD)


    /// Helper Methods ==============================================
    /* Marker */
    /**
     * @brief Unmark all nodes in the forest. This is usually used to initialize 
     * for counting the marked nodes or sweeping the unmarked nodes.
     * 
     */
    void unmark();
    /**
     * @brief Mark all the nonterminal nodes reachable from the given root node 
     * in the forest.
     * 
     * @param root          The root node handle.
     */
    void markNodes(NodeHandle root);
    /**
     * @brief Mark all the nonterminal nodes reachable from all root nodes in 
     * the forest.
     * 
     */
    void markAllRoots();


    /// =============================================================
        ForestSetting       setting;        // Specification setting of this forest.
        NodeManager*        nodeMan;        // Node manager.
        UniqueTable*        uniqueTable;    // Unique table.
        Root*               roots;          // Registry of Root edges.
        Statistics*         stats;          // Performance measurement.

};


#endif