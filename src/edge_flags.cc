#include "edge_flags.h"

// ******************************************************************
// *                                                                *
// *                                                                *
// *                          Flags methods                         *
// *                                                                *
// *                                                                *
// ******************************************************************

BRAVE_DD::Flags::Flags(SwapSet swap, CompSet comp)
{
    swapType = swap;
    compType = comp;
}
BRAVE_DD::Flags::~Flags()
{
    //
}