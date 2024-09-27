#include "node_manager.h"
#include "forest.h"


using namespace BRAVE_DD;
// ******************************************************************
// *                                                                *
// *                                                                *
// *                       SubManager methods                       *
// *                                                                *
// *                                                                *
// ******************************************************************

NodeManager::SubManager::SubManager(Forest *f):parent(f)
{
    sizeIndex = 0;
    nodes = std::vector<PackedNode>(PRIMES[sizeIndex], PackedNode(f->getSetting()));
    recycled = 0;
    firstUnalloc = 0;
    freeList = 0;
    numFrees = 0;   // do we really need this?
}
NodeManager::SubManager::~SubManager()
{
    //
}
// ******************************************************************
// *                                                                *
// *                                                                *
// *                      NodeManager methods                       *
// *                                                                *
// *                                                                *
// ******************************************************************

NodeManager::NodeManager(Forest *f):parent(f)
{
    uint16_t lvls = f->getSetting().getNumVars();
    chunks = std::vector<SubManager>(lvls, SubManager(f));
}
NodeManager::~NodeManager()
{
    //
}