#include "edge.h"

using namespace BRAVE_DD;
// ******************************************************************
// *                                                                *
// *                                                                *
// *                      Value  methods                        *
// *                                                                *
// *                                                                *
// ******************************************************************

Value::Value()
{
    valueType = INT;
    intValue = 0;
}
Value::Value(int i)
{
    valueType = INT;
    intValue = i;
}
Value::Value(long l)
{
    valueType = LONG;
    longValue = l;
}
Value::Value(double d)
{
    valueType = DOUBLE;
    doubleValue = d;
}
Value::Value(float f)
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
    value = Value(0);
}
Edge::Edge(const Edge &e)
{
    handle = e.handle;
    value = e.value;
}
Edge::Edge(const EdgeHandle h, Value val):value(val)
{
    //
    handle = h;
}
Edge::~Edge()
{

}
