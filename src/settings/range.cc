#include "range.h"

using namespace BRAVE_DD;
// ******************************************************************
// *                                                                *
// *                                                                *
// *                          Range methods                         *
// *                                                                *
// *                                                                *
// ******************************************************************

Range::Range(const RangeType range, const ValueType val)
{
    //
    rangeType = range;
    valueType = val;
    maxRange = 1;   // default for boolean; used for complement
    negInf = 0;
    posInf = 0;
    unDef = 0;

}
Range::Range(unsigned long size)
{
    //
    rangeType = FINITE;
    valueType = LONG;
    maxRange = size;
    negInf = 0;
    posInf = 0;
    unDef = 0;
}
Range::~Range()
{
    //
}