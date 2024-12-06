#include "forest.h"

using namespace BRAVE_DD;
// ******************************************************************
// *                                                                *
// *                                                                *
// *                        Forest methods                          *
// *                                                                *
// *                                                                *
// ******************************************************************

Forest::Forest(const ForestSetting& s):setting(s)
{
    /* Check consistency */
    if (!s.checkConsistency()) {
        std::cout << "[BRAVE_DD] Error!\t The ForestSetting consistency check failed!"<< std::endl;
        setting.exportSetting(std::cerr);
        exit(1);
    }
    nodeSize = setting.nodeSize();
    nodeMan = new NodeManager(this);
    uniqueTable = new UniqueTable(this);
    stats = new Statistics();
}
Forest::~Forest()
{
    delete nodeMan;
    delete uniqueTable;
    delete stats;
}
/***************************** Cardinality **********************/
uint64_t Forest::count(Func func, int val)
{
    uint64_t num = 0;
    // TBD
    return num;
}
/****************************** I/O *****************************/
void Forest::exportFunc(std::ostream& out, FuncArray func)
{
    //
}
void Forest::exportForest(std::ostream& out)
{
    //
}
FuncArray Forest::importFunc(std::istream& in)
{
    //
    FuncArray result;
    // TBD
    return result;
}
void Forest::importForest(std::istream& in)
{
    //
}
/************************* Reduction ****************************/
Edge Forest::reduceNode(const uint16_t nodeLevel, const std::vector<Edge>& down)
{
    Edge reduced;
    /* check if the node matches an illegal pattern */

    return reduced;

}

Edge Forest::mergeEdge(const uint16_t beginLevel, const uint16_t mergeLevel, const EdgeLabel label, const Edge& reduced, const Value& value)
{
    Edge merged;
    /* check whether push-up or push-down, shortenI or shortenX */
    // TBD
    return merged;
}

Edge Forest::reduceEdge(const uint16_t beginLevel, const EdgeLabel label, const uint16_t nodeLevel, const std::vector<Edge>& down, const Value& value)
{
    /* check level */
    if (beginLevel < nodeLevel) {
        std::cout << "[BRAVE_DD] ERROR!\t Invalid level for incoming edge or target node!" << std::endl;
        exit(0);
    }
    /* check number of child */
    if ((setting.isRelation() && down.size() != 4) || (!setting.isRelation() && down.size() != 2)) {
        std::cout << "[BRAVE_DD] ERROR!\t Incorrect number of child edges!" << std::endl;
        exit(0);
    }
    /* copy the children info */
    std::vector<Edge> child = down;
    /* push the flags or value down */
    CompSet ct = setting.getCompType();
    if (ct == COMP && unpackComp(label)) {                          // complement
        for (size_t i=0; i<child.size(); i++) child[i].complement();
    }
    SwapSet st = setting.getSwapType();
    if (!setting.isRelation() && unpackSwap(label)) {           // set: swap-one or swap-all
        if (st == ONE) {                                            // swap-one
            SWAP(child[0], child[1]);
        } else if (st == ALL) {                                     // swap-all
            SWAP(child[0], child[1]);
            child[0].swap();
            child[1].swap();
        }
    } else if (setting.isRelation()) {                          // relation: swap-from, swap-to, swap-from_to
        if ((st == FROM || st == FROM_TO) && unpackSwap(label)) {   // swap "from"
            SWAP(child[0], child[2]);
            SWAP(child[1], child[3]);
        }
        if ((st == TO || st == FROM_TO) && unpackSwapTo(label)) {   // swap "to"
            // to
            SWAP(child[0], child[1]);
            SWAP(child[2], child[3]);
        }
    }
    /* reduce node */
    Edge reduced;
    reduced = reduceNode(nodeLevel, child);    // this will take care of value on edge
    /* incoming edge rule for merge */
    EdgeLabel mergeLabel = 0;
    packRule(mergeLabel, unpackRule(label));
    /* merge incoming edge with reduced node */
    if (setting.getEncodeMechanism() == TERMINAL) {
        reduced = mergeEdge(beginLevel, nodeLevel, mergeLabel, reduced);
    } else {
        reduced = mergeEdge(beginLevel, nodeLevel, mergeLabel, reduced, value);
    }
    return reduced;
}

void Forest::markNodes(const Edge& edge) const
{
    char numChild = (setting.isRelation()) ? 4 : 2;
    if (edge.getNodeLevel() > 0) {
        if (!getNode(edge).isMarked()) {
            getNode(edge).mark();
            for (char i=0; i<numChild; i++) {
                markNodes(getChildEdge(edge.getNodeLevel(), edge.getNodeHandle(), i));
            }
        }
    }
}
