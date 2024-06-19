#include "range.h"

// ******************************************************************
// *                                                                *
// *                                                                *
// *                          Range methods                         *
// *                                                                *
// *                                                                *
// ******************************************************************

BRAVE_DD::Range::Range(const RangeType range, const ValueType val)
{
    //
    rangeType = range;
    valueType = val;
    // Check the consistency

}
BRAVE_DD::Range::Range(unsigned long size)
{
    //
    rangeType = FINITE;
    valueType = LONG;
    maxRange = size;
}
BRAVE_DD::Range::~Range()
{
    //
}