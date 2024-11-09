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
    try
    {
        /* Check consistency */
        s.checkConsistency();
    }
    catch(const error& e)
    {
        std::cerr << e.what() << '\n';
        exit(e.getCode());
    }
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

Edge Forest::reduceEdge(uint16_t beginLevel, EdgeLabel label, uint16_t nodeLevel, std::vector<Edge> child)
{
    // check lvl >= P.level, and out is not null
    // push the flags or value down, TBD
    // Edge reduced;
    // reduceNode(P, &reduced);
    // mergeEdge(lvl, P.level, label, &reduced, out);
    Edge res(child[0]);
    return res;
}
