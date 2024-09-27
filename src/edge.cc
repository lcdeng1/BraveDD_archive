#include "edge.h"

using namespace BRAVE_DD;
// ******************************************************************
// *                                                                *
// *                                                                *
// *                      EdgeValue  methods                        *
// *                                                                *
// *                                                                *
// ******************************************************************

EdgeValue::EdgeValue()
{
    valueType = INT;
    intValue = 0;
}
EdgeValue::EdgeValue(int i)
{
    valueType = INT;
    intValue = i;
}
EdgeValue::EdgeValue(long l)
{
    valueType = LONG;
    longValue = l;
}
EdgeValue::EdgeValue(double d)
{
    valueType = DOUBLE;
    doubleValue = d;
}
EdgeValue::EdgeValue(float f)
{
    valueType = FLOAT;
    floatValue = f;
}

// ******************************************************************
// *                                                                *
// *                                                                *
// *                         Edge  methods                          *
// *                                                                *
// *                                                                *
// ******************************************************************

Edge::Edge()
{
    handle = 0;
    value = EdgeValue(0);
}
Edge::Edge(const Edge &e)
{
    //
}
Edge::Edge(const EdgeHandle h, EdgeValue val):value(val)
{
    //
    handle = h;
}
Edge::~Edge()
{

}
