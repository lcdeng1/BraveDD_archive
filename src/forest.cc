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

/**************************** Make edge *************************/
Func Forest::constant(int val)
{
    Func result;
    // TBD
    return result;
}
Func Forest::constant(long val)
{
    Func result;
    // TBD
    return result;
}
Func Forest::constant(float val)
{
    Func result;
    // TBD
    return result;
}
Func Forest::constant(double val)
{
    Func result;
    // TBD
    return result;
}
Func Forest::variable(uint16_t lvl)
{
    Func result;
    // TBD
    return result;
}
Func Forest::variable(uint16_t lvl, int low, int high)
{
    Func result;
    // TBD
    return result;
}

/****************************** I/O *****************************/
void Forest::exportFunc(std::ostream& out, FuncSet func)
{
    //
}
void Forest::exportForest(std::ostream& out)
{
    //
}
FuncSet Forest::importFunc(std::istream& in)
{
    //
    FuncSet result;
    // TBD
    return result;
}
void Forest::importForest(std::istream& in)
{
    //
}