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
    //
}
EdgeValue::EdgeValue(int i)
{
    //
}
EdgeValue::EdgeValue(long i)
{
    //
}
EdgeValue::EdgeValue(double d)
{
    //
}
EdgeValue::EdgeValue(float f)
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

Edge::Edge()
{
    handle = 0;
    value = 0;
}
Edge::Edge(const Edge &e)
{
    //
}
Edge::~Edge()
{

}
