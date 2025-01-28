#include "operations_generator.h"

namespace BRAVE_DD {
    UnaryList UOPs;
    BinaryList BOPs;
}

using namespace BRAVE_DD;
// ******************************************************************
// *                                                                *
// *                            Front End                           *
// *                                                                *
// ******************************************************************
// Unary operations
UnaryOperation* BRAVE_DD::COPY(Forest* arg, Forest* res)
{
    if (!arg) return nullptr;
    UnaryOperation* uop = UOPs.find(UnaryOperationType::UOP_COPY, arg, res);
    if (uop) return uop;
    return UOPs.add(new UnaryOperation(UOPs, UnaryOperationType::UOP_COPY, arg, res));
}
UnaryOperation* BRAVE_DD::CARDINALITY(Forest* arg, OpndType res)
{
    if (!arg) return nullptr;
    UnaryOperation* uop = UOPs.find(UnaryOperationType::UOP_CARDINALITY, arg, res);
    if (uop) return uop;
    return UOPs.add(new UnaryOperation(UOPs, UnaryOperationType::UOP_CARDINALITY, arg, res));
}
UnaryOperation* BRAVE_DD::COMPLEMENT(Forest* arg, Forest* res)
{
    if (!arg) return nullptr;
    UnaryOperation* uop = UOPs.find(UnaryOperationType::UOP_COMPLEMENT, arg, res);
    if (uop) return uop;
    return UOPs.add(new UnaryOperation(UOPs, UnaryOperationType::UOP_COMPLEMENT, arg, res));
}


// ... TBD

// Binary operations
BinaryOperation* BRAVE_DD::UNION(Forest* arg1, Forest* arg2, Forest* res)
{
    //
}
BinaryOperation* BRAVE_DD::INTERSECTION(Forest* arg1, Forest* arg2, Forest* res)
{
    //
}