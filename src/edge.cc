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

void Value::print(std::ostream& out, int format) const
{
    if (format == 0) {
        //
    } else {
        // more format TBD
    }
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

void Edge::print(std::ostream& out, int format) const
{
    printEdgeHandle(handle, out, format);
    // value?
    if (format == 0) {
        //
    } else {
        // more format TBD
    }
}
