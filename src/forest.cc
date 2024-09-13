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