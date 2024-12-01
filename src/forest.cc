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
Edge Forest::reduceNode(uint16_t nodeLevel, std::vector<Edge>& child)
{
    //
}

Edge Forest::reduceEdge(uint16_t beginLevel, EdgeLabel label, uint16_t nodeLevel, std::vector<Edge>& child)
{
    /* check level */
    if (beginLevel < nodeLevel) {
        std::cout << "[BRAVE_DD] ERROR!\t Invalid level for incoming edge or target node!" << std::endl;
        exit(0);
    }
    /* check number of child */
    if ((setting.isRelation() && child.size() != 4) || (!setting.isRelation() && child.size() != 2)) {
        std::cout << "[BRAVE_DD] ERROR!\t Incorrect number of child edges!" << std::endl;
        exit(0);
    }
    /* push the flags or value down */
    SwapSet st = setting.getSwapType();
    if (st == ONE) {
        // swap-one
    } else if (st == ALL) {
        //
    }


    // the reduced node
    
    // Edge reduced;
    // reduceNode(P, &reduced);
    // mergeEdge(lvl, P.level, label, &reduced, out);
    Edge res(child[0]);
    return res;
}
