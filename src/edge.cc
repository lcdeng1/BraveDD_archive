#include "edge.h"

// ******************************************************************
// *                                                                *
// *                                                                *
// *                      EdgeValue  methods                        *
// *                                                                *
// *                                                                *
// ******************************************************************

BRAVE_DD::EdgeValue::EdgeValue()
{
    //
}
BRAVE_DD::EdgeValue::EdgeValue(int i)
{
    //
}
BRAVE_DD::EdgeValue::EdgeValue(double d)
{
    //
}
BRAVE_DD::EdgeValue::EdgeValue(float f)
{
    //
}

// ******************************************************************
// *                                                                *
// *                                                                *
// *                         Edge  methods                          *
// *                                                                *
// *                                                                *
// ******************************************************************

BRAVE_DD::Edge::Edge()
{
    handle = 0;
    value = 0;
    display = "<>";
}
BRAVE_DD::Edge::Edge(const Edge &e)
{
    //
}
BRAVE_DD::Edge::~Edge()
{

}

// ******************************************************************
// *                                                                *
// *                                                                *
// *                         Root  methods                          *
// *                                                                *
// *                                                                *
// ******************************************************************
BRAVE_DD::Root::Root()
{
    //
}
BRAVE_DD::Root::Root(Forest* f)
{
    //
}
BRAVE_DD::Root::Root(Forest* f, const Edge& e)
:edge(e)
{
    //
}
BRAVE_DD::Root::~Root()
{
    //
}