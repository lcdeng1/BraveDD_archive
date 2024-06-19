#include "edge.h"

// ******************************************************************
// *                                                                *
// *                                                                *
// *                         Edge  methods                          *
// *                                                                *
// *                                                                *
// ******************************************************************

BRAVE_DD::Edge::Edge()
{
    targetNode = 0;
    prevEdge = nullptr;
    nextEdge = nullptr;
}
BRAVE_DD::Edge::Edge(Forest* parent)
{
    //
    targetNode = 0;
    parentForest = parent;
    nextEdge = 0;
}
BRAVE_DD::Edge::Edge(const Edge &e)
{
    //
}
BRAVE_DD::Edge::~Edge()
{

}