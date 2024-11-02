#ifndef BRAVE_DD_SATURATION_H
#define BRAVE_DD_SATURATION_H

#include "../defines.h"
#include "operation.h"

namespace BRAVE_DD {
    class SaturationOperation;
    class SaturationList;
};

// ******************************************************************
// *                                                                *
// *                SaturationOperation  class                       *
// *                                                                *
// ******************************************************************

class BRAVE_DD::SaturationOperation : public Operation {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    SaturationOperation();
    /*-------------------------------------------------------------*/
    protected:
    /*-------------------------------------------------------------*/
    virtual ~SaturationOperation();
    // computing tables TBD

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
};

#endif