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
void BRAVE_DD::Forest::makeConstant(long val, unsigned lvl, Edge* out)
{
    //
}
void BRAVE_DD::Forest::makeVariable(unsigned lvl, Edge* out)
{
    //
}