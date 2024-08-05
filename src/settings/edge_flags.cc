#include "edge_flags.h"

using namespace BRAVE_DD;
// ******************************************************************
// *                                                                *
// *                                                                *
// *                          Flags methods                         *
// *                                                                *
// *                                                                *
// ******************************************************************

Flags::Flags(SwapSet swap, CompSet comp)
{
    swapType = swap;
    compType = comp;
}
Flags::~Flags()
{
    //
}