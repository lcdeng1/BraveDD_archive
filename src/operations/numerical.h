#ifndef BRAVE_DD_NUMERICAL_H
#define BRAVE_DD_NUMERICAL_H

#include "../defines.h"
#include "operation.h"

namespace BRAVE_DD {
    class NumericalOperation;
    class NumericalList;
};

// ******************************************************************
// *                                                                *
// *                NumericalOperation  class                       *
// *                                                                *
// ******************************************************************

class BRAVE_DD::NumericalOperation : public Operation {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    NumericalOperation();
    /*-------------------------------------------------------------*/
    protected:
    /*-------------------------------------------------------------*/
    virtual ~NumericalOperation();
    // computing tables TBD

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
};

#endif