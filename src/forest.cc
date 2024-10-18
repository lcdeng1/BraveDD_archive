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
    //
    // setting.~ForestSetting();
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

void Forest::reduceNode(uint16_t lvl, Node& P, Edge* out)
{
    //
}

void Forest::mergeEdge(uint16_t lvl1, uint16_t lvl2, EdgeLabel label, Edge* reduced, Edge* out)
{
    //
}

void Forest::reduceEdge(uint16_t lvl, EdgeLabel label, Node& P, Edge* out)
{
    // check lvl >= P.level, and out is not null
    // push the flags or value down, TBD
    // Edge reduced;
    // reduceNode(P, &reduced);
    // mergeEdge(lvl, P.level, label, &reduced, out);

}

/* Expert function for evaluate TBD */
bool Forest::evaluateRecursive(uint16_t n, Func func, std::vector<bool> assignment)
{
    //
    bool final = 0;

    return final;
}
/* Expert function for union assignments TBD */
Func Forest::unionAssignmentRecursive(uint16_t lvl, Func& func, std::vector<bool> assignment, uint64_t outcome)
{
    //
    return func;
}
