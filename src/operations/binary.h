#ifndef BRAVE_DD_BINARY_H
#define BRAVE_DD_BINARY_H

#include "../defines.h"
#include "operation.h"

namespace BRAVE_DD {
    class BinaryOperation;
    class BinaryList;
};

// ******************************************************************
// *                                                                *
// *                   BinaryOperation  class                       *
// *                                                                *
// ******************************************************************

class BRAVE_DD::BinaryOperation : public Operation {
    /*-------------------------------------------------------------*/
    public:
    /*-------------------------------------------------------------*/
    BinaryOperation();
    /*-------------------------------------------------------------*/
    protected:
    /*-------------------------------------------------------------*/
    virtual ~BinaryOperation();
    // computing tables TBD

    /*-------------------------------------------------------------*/
    private:
    /*-------------------------------------------------------------*/
};

#endif