#include "statistics.h"

// ******************************************************************
// *                                                                *
// *                                                                *
// *                      statistics methods                        *
// *                                                                *
// *                                                                *
// ******************************************************************

BRAVE_DD::Statistics::Statistics()
{
    //
    activeNodes = 0;
    peakNodes = 0;
    numOps = 0;
    numOpsTerms = 0;
    numHitsCT = 0;
}
BRAVE_DD::Statistics::~Statistics()
{
    //
}