#ifndef BRAVE_DD_FOREST_H
#define BRAVE_DD_FOREST_H

#include "defines.h"
#include "setting.h"
#include "edge.h"
#include "terminal.h"
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
 * A data structure for managing collections of functions (for sets, 
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
    /**
     * @brief Get the stored node from the NodeManager by giving its level and node handle.
     * 
     * @param level         The level of the node.
     * @param handle        The handle of the node.
     * @return Node         – Output the node stored in NodeManager.
     */
    inline Node& getNode(const uint16_t level, const NodeHandle& handle) const {
        return nodeMan->getNodeFromHandle(level, handle);
    }

    /**
     * @brief Get the stored target node from the NodeManager by giving its incoming edge handle.
     * 
     * @param edge          The given incoming edge handle
     * @return Node         – Output the targer node stored in NodeManager.
     */
    inline Node& getNode(const EdgeHandle& edge) const {
        return getNode(unpackLevel(edge), unpackNode(edge));
    }
    
    /**
     * @brief Get the stored target Node from the NodeManager by giving its incoming edge.
     * 
     * @param edge          The given incoming edge.
     * @return Node         – Output the targer node stored in NodeManager.
     */
    inline Node& getNode(const Edge& edge) const {
        return getNode(edge.getNodeLevel(), edge.getNodeHandle());
    }

    /**
     * @brief Get the stored node's next pointer (NodeHandle of another unique node at the same level having 
     * the same hash value) by giving its level and node handle.
     * 
     * @param level         The given node level.
     * @param handle        The given node handle.
     * @return NodeHandle   - Output the next pointer.
     */
    inline NodeHandle getNodeNext(const uint16_t level, const NodeHandle handle) const {
        return nodeMan->getNodeFromHandle(level, handle).getNext();
    }

    /**
     * @brief Set the stored nodes's next pointer.
     * 
     * @param level         The givien node level.
     * @param handle        The givien node handle.
     * @param next          The next handle to be set.
     */
    inline void setNodeNext(const uint16_t level, const NodeHandle handle, const NodeHandle next) {
        nodeMan->getNodeFromHandle(level, handle).setNext(next);
    }

    /**
     * @brief Compute and get the hash value of a node stored in NodeManager, by giving the node's level and node handle.
     * 
     * @param level         The given node level.
     * @param handle        The given node handle.
     * @return uint64_t     - Output the node's hash value.
     */
    inline uint64_t getNodeHash(const uint16_t level, const NodeHandle handle) const {
        return nodeMan->getNodeFromHandle(level, handle).hash(nodeSize);
    }

    /**
     * @brief Store a node in NodeManager and return its node handle, by giving the node's level and itself.
     * Note: This does not guarantee that the node is uniquely stored.
     * 
     * @param level 
     * @param node 
     * @return NodeHandle 
     */
    inline NodeHandle obtainFreeNodeHandle(const uint16_t level, const Node& node) {
        return nodeMan->getFreeNodeHandle(level, node);
    }
    
    /**
     * @brief Uniquely store a node and get its node handle by giving its level and itself.
     * 
     * @param level         The level of the node.
     * @param node          The node to insert.
     * @return NodeHandle   - Output the unique node handle stored in UniqueTable.
     */
    inline NodeHandle insertNode(const uint16_t level, const Node& node) {
        return uniqueTable->insert(level, node);
    }

    /**
     * @brief Get the child edge label (including information of rules and flags) of a node, by giving the node's 
     * incoming edge handle and the index of child edge.
     * 
     * @param handle        The incoming edge handle.
     * @param index         The index of target node's child.
     * @return EdgeLabel    - Output the target node's child edge label.
     */
    inline EdgeLabel getChildLabel(NodeHandle handle, int index) const {
        // TBD
        return 0;
    }

    /**
     * @brief Get the child edge of a node, by giving the node's incoming edge and the index of child edge.
     * 
     * @param edge          The incoming edge.
     * @param index         The index of target node's child.
     * @return Edge         - Output the target node's child edge.
     */
    inline Edge getChildEdge(const Edge& edge, const int index) const {
        // TBD
        Edge res(edge);
        return res;
    }

    /************************* Reduction ****************************/
    /**
     * @brief Reduce and return an edge pointing to a unique reduced node stored in NodeManager, by giving the beginning level 
     * of an unreduced incoming edge, its unreduced edge label, and unreduced target node (its level, child edges).
     * 
     * @param beginLevel    The beginning level of the unreduced incoming edge.
     * @param label         The unreduced incoming edge label.
     * @param nodeLevel     The level of the unreduced target node.
     * @param child         The vector of child edges of the unreduced target node.
     * @return Edge         - Output: the reduced edge pointing to a reduced node uniquely stored.
     */
    Edge reduceEdge(uint16_t beginLevel, EdgeLabel label, uint16_t nodeLevel, std::vector<Edge> child);




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
    inline void sweepNodeMan(uint16_t level) {nodeMan->sweep(level);}
    void markSweep();
    // TBD

    /************************* Statistics Information ***************/
    inline uint32_t getNodeManUsed(const uint16_t level) const {
        return nodeMan->numUsed(level);
    }
    inline uint32_t getNodeManAlloc(const uint16_t level) const {
        return nodeMan->numAlloc(level);
    }
    inline uint32_t getUTEntriesNum(const uint16_t level) const {
        return uniqueTable->getNumEntries(level);
    }
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
    /**
     * @brief Normalize a node, 
     * Normalize the given node "P". EdgeLabel "*out" specifies 
     * the incoming edge rule/value and flags, which may be changed by the normalization.
     * 
     * @param lvl           The given node level.
     * @param P             The given node waiting for normalization.
     * @param out           Output: edge label (rule/value, flags).
     */
    void normalizeNode(uint16_t lvl, Node& P, EdgeLabel& out);
    /**
     * @brief Reduce the given node "P" by checking the forbidden patterns 
     * of nodes. Edge "*out" will be written the long edge that represent 
     * node "*P".
     * 
     * @param lvl           The given node level.
     * @param P             The given node waiting for reduction.
     * @param out           Output: reduced edge (label, target node handle).
     */
    void reduceNode(uint16_t lvl, Node& P, Edge& out);
    /**
     * @brief Merge the incoming edge having EdgeLabel "label", which is respect of 
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
    void mergeEdge(uint16_t lvl1, uint16_t lvl2, EdgeLabel label, const Edge& reduced, Edge& out);
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
    friend class NodeManager;
    friend class UniqueTable;
        int                 nodeSize;       // Number of uint32 slots for one Node storage.
        ForestSetting       setting;        // Specification setting of this forest.
        NodeManager*        nodeMan;        // Node manager.
        UniqueTable*        uniqueTable;    // Unique table.
        Func*               funcs;          // Registry of Func edges.
        FuncArray*          funcSets;       // Sets of Func used for I/O.
        Statistics*         stats;          // Performance measurement.

};


#endif