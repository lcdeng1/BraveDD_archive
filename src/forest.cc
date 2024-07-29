#include "forest.h"

// ******************************************************************
// *                                                                *
// *                                                                *
// *                        Forest methods                          *
// *                                                                *
// *                                                                *
// ******************************************************************

BRAVE_DD::Forest::Forest(const ForestSetting& s):setting(s)
{
    try
    {
        /* Check consistency */
        s.checkConsistency();
    }
    catch(const BRAVE_DD::error& e)
    {
        std::cerr << e.what() << '\n';
        exit(e.getCode());
    }

    nodeMan = new NodeManager(this);
    uniqueTable = new UniqueTable(this);
    stats = new Statistics();

}
BRAVE_DD::Forest::~Forest()
{
    //
    // setting.~ForestSetting();
    delete nodeMan;
    delete uniqueTable;
    delete stats;
}

/**************************** Make edge *************************/
BRAVE_DD::Root BRAVE_DD::Forest::constant(int val)
{
    BRAVE_DD::Root result(this);
    // TBD
    return result;
}
BRAVE_DD::Root BRAVE_DD::Forest::variable(int lvl)
{
    BRAVE_DD::Root result(this);
    // TBD
    return result;
}