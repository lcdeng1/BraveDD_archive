#ifndef BRAVE_DD_STATISTICS_H
#define BRAVE_DD_STATISTICS_H

#include "defines.h"

namespace BRAVE_DD {
    class Statistics;
}; // namespace BRAVE_DD

// ******************************************************************
// *                                                                *
// *                                                                *
// *                      statistics class                          *
// *                                                                *
// *                                                                *
// ******************************************************************
/**
 * @brief Statistics class.
 * 
 * A data structure for managing collection of various stats for 
 * performance measurement.
 * 
 */
class BRAVE_DD::Statistics {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    Statistics();
    ~Statistics();
    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
    uint64_t activeNodes;       // Current number of connected nodes.
    uint64_t peakNodes;         // Peak number of active nodes.

    uint64_t numOps;            // Number of operation calls.
    uint64_t numOpsTerms;       // Number of operation calls terminal case.
    uint64_t numHitsCT;         // Number of hits in computing table.
    // more numbers... TBD 
};
#endif