#include "forest.h"

// ******************************************************************
// *                                                                *
// *                                                                *
// *                        Forest methods                          *
// *                                                                *
// *                                                                *
// ******************************************************************

BRAVE_DD::Forest::Forest(ForestSetting* s)
{
    //
    setting = s;
    nodeMan = new NodeManager(this);

}
BRAVE_DD::Forest::~Forest()
{
    //
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