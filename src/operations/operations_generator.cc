#include "operations_generator.h"

namespace BRAVE_DD {
    UnaryList       UOPs;
    BinaryList      BOPs;
    SaturationList  SOPs;
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
    return UOPs.add(new UnaryOperation(UnaryOperationType::UOP_COPY, arg, res));
}

UnaryOperation* BRAVE_DD::CARDINALITY(Forest* arg, OpndType res)
{
    if (!arg) return nullptr;
    UnaryOperation* uop = UOPs.find(UnaryOperationType::UOP_CARDINALITY, arg, res);
    if (uop) return uop;
    return UOPs.add(new UnaryOperation(UnaryOperationType::UOP_CARDINALITY, arg, res));
}

UnaryOperation* BRAVE_DD::COMPLEMENT(Forest* arg, Forest* res)
{
    if (!arg) return nullptr;
    UnaryOperation* uop = UOPs.find(UnaryOperationType::UOP_COMPLEMENT, arg, res);
    if (uop) return uop;
    return UOPs.add(new UnaryOperation(UnaryOperationType::UOP_COMPLEMENT, arg, res));
}

UnaryOperation* BRAVE_DD::CONCRETIZE_RST(Forest* arg, Forest* res)
{
    if (!arg) return nullptr;
    UnaryOperation* uop = UOPs.find(UnaryOperationType::UOP_CONCRETIZE_RST, arg, res);
    if (uop) return uop;
    return UOPs.add(new UnaryOperation(UnaryOperationType::UOP_CONCRETIZE_RST, arg, res));
}
UnaryOperation* BRAVE_DD::CONCRETIZE_OSM(Forest* arg, Forest* res)
{
    if (!arg) return nullptr;
    UnaryOperation* uop = UOPs.find(UnaryOperationType::UOP_CONCRETIZE_OSM, arg, res);
    if (uop) return uop;
    return UOPs.add(new UnaryOperation(UnaryOperationType::UOP_CONCRETIZE_OSM, arg, res));
}
UnaryOperation* BRAVE_DD::CONCRETIZE_TSM(Forest* arg, Forest* res)
{
    if (!arg) return nullptr;
    UnaryOperation* uop = UOPs.find(UnaryOperationType::UOP_CONCRETIZE_TSM, arg, res);
    if (uop) return uop;
    return UOPs.add(new UnaryOperation(UnaryOperationType::UOP_CONCRETIZE_TSM, arg, res));
}


// ... TBD

// Binary operations
BinaryOperation* BRAVE_DD::UNION(Forest* arg1, Forest* arg2, Forest* res)
{
    if (!arg1 || !arg2) return nullptr;
    // commute
    if (arg1 > arg2) SWAP(arg1, arg2);
    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_UNION, arg1, arg2, res);
    if (bop) return bop;
    bop = new BinaryOperation(BinaryOperationType::BOP_UNION, arg1, arg2, res);
    return BOPs.add(bop);
}

BinaryOperation* BRAVE_DD::INTERSECTION(Forest* arg1, Forest* arg2, Forest* res)
{
    if (!arg1 || !arg2) return nullptr;
    // commute
    if (arg1 > arg2) SWAP(arg1, arg2);
    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_INTERSECTION, arg1, arg2, res);
    if (bop) return bop;
    bop = new BinaryOperation(BinaryOperationType::BOP_INTERSECTION, arg1, arg2, res);
    return BOPs.add(bop);
}

BinaryOperation* BRAVE_DD::DIFFERENCE(Forest* arg1, Forest* arg2, Forest* res)
{
    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_DIFFERENCE, arg1, arg2, res);
    if (bop) return bop;
    bop = new BinaryOperation(BinaryOperationType::BOP_DIFFERENCE, arg1, arg2, res);
    return BOPs.add(bop);
}


BinaryOperation* BRAVE_DD::MINIMUM(Forest* arg1, Forest* arg2, Forest* res)
{
    if (!arg1) return nullptr;
    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_MINIMUM, arg1, arg2, res);
    if (bop) return bop;
    bop = new BinaryOperation(BinaryOperationType::BOP_MINIMUM, arg1, arg2, res);
    return BOPs.add(bop);
}

BinaryOperation* BRAVE_DD::MINIMUM(Forest* arg1, OpndType arg2, Forest* res)
{
    if (!arg1) return nullptr;
    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_MINIMUM, arg1, arg2, res);
    if (bop) return bop;
    bop = new BinaryOperation(BinaryOperationType::BOP_MINIMUM, arg1, arg2, res);
    return BOPs.add(bop);
}

BinaryOperation* BRAVE_DD::MAXIMUM(Forest* arg1, Forest* arg2, Forest* res)
{
    if (!arg1) return nullptr;
    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_MAXIMUM, arg1, arg2, res);
    if (bop) return bop;
    bop = new BinaryOperation(BinaryOperationType::BOP_MAXIMUM, arg1, arg2, res);
    return BOPs.add(bop);
}

BinaryOperation* BRAVE_DD::MAXIMUM(Forest* arg1, OpndType arg2, Forest* res)
{
    if (!arg1) return nullptr;
    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_MAXIMUM, arg1, arg2, res);
    if (bop) return bop;
    bop = new BinaryOperation(BinaryOperationType::BOP_MAXIMUM, arg1, arg2, res);
    return BOPs.add(bop);
}

BinaryOperation* BRAVE_DD::PLUS(Forest* arg1, Forest* arg2, Forest* res)
{
    if (!arg1) return nullptr;
    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_PLUS, arg1, arg2, res);
    if (bop) return bop;
    bop = new BinaryOperation(BinaryOperationType::BOP_PLUS, arg1, arg2, res);
    return BOPs.add(bop);
}

BinaryOperation* BRAVE_DD::MINUS(Forest* arg1, Forest* arg2, Forest* res)
{
    if (!arg1) return nullptr;
    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_MINUS, arg1, arg2, res);
    if (bop) return bop;
    bop = new BinaryOperation(BinaryOperationType::BOP_MINUS, arg1, arg2, res);
    return BOPs.add(bop);
}

BinaryOperation* BRAVE_DD::MULTIPLY(Forest* arg1, Forest* arg2, Forest* res)
{
    if (!arg1) return nullptr;
    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_MULTIPLY, arg1, arg2, res);
    if (bop) return bop;
    bop = new BinaryOperation(BinaryOperationType::BOP_MULTIPLY, arg1, arg2, res);
    return BOPs.add(bop);
}

BinaryOperation* BRAVE_DD::DIVIDE(Forest* arg1, Forest* arg2, Forest* res)
{
    if (!arg1) return nullptr;
    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_DIVIDE, arg1, arg2, res);
    if (bop) return bop;
    bop = new BinaryOperation(BinaryOperationType::BOP_DIVIDE, arg1, arg2, res);
    return BOPs.add(bop);
}

BinaryOperation* BRAVE_DD::EQUAL(Forest* arg1, Forest* arg2, Forest* res)
{
    if (!arg1) return nullptr;
    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_EQUAL, arg1, arg2, res);
    if (bop) return bop;
    bop = new BinaryOperation(BinaryOperationType::BOP_EQUAL, arg1, arg2, res);
    return BOPs.add(bop);
}

BinaryOperation* BRAVE_DD::NOT_EQUAL(Forest* arg1, Forest* arg2, Forest* res)
{
    if (!arg1) return nullptr;
    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_NOTEQUAL, arg1, arg2, res);
    if (bop) return bop;
    bop = new BinaryOperation(BinaryOperationType::BOP_NOTEQUAL, arg1, arg2, res);
    return BOPs.add(bop);
}

BinaryOperation* BRAVE_DD::LESS_THAN(Forest* arg1, Forest* arg2, Forest* res)
{
    if (!arg1) return nullptr;
    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_LESSTHAN, arg1, arg2, res);
    if (bop) return bop;
    bop = new BinaryOperation(BinaryOperationType::BOP_LESSTHAN, arg1, arg2, res);
    return BOPs.add(bop);
}

BinaryOperation* BRAVE_DD::LESS_THAN_EQUAL(Forest* arg1, Forest* arg2, Forest* res)
{
    if (!arg1) return nullptr;
    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_LESSTHANEQ, arg1, arg2, res);
    if (bop) return bop;
    bop = new BinaryOperation(BinaryOperationType::BOP_LESSTHANEQ, arg1, arg2, res);
    return BOPs.add(bop);
}

BinaryOperation* BRAVE_DD::GREATER_THAN(Forest* arg1, Forest* arg2, Forest* res)
{
    if (!arg1) return nullptr;
    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_GREATERTHAN, arg1, arg2, res);
    if (bop) return bop;
    bop = new BinaryOperation(BinaryOperationType::BOP_GREATERTHAN, arg1, arg2, res);
    return BOPs.add(bop);
}

BinaryOperation* BRAVE_DD::GREATER_THAN_EQUAL(Forest* arg1, Forest* arg2, Forest* res)
{
    if (!arg1) return nullptr;
    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_GREATERTHANEQ, arg1, arg2, res);
    if (bop) return bop;
    bop = new BinaryOperation(BinaryOperationType::BOP_GREATERTHANEQ, arg1, arg2, res);
    return BOPs.add(bop);
}

BinaryOperation* BRAVE_DD::PRE_IMAGE(Forest* arg1, Forest* arg2, Forest* res)
{
    if (!arg1 || !arg2) return nullptr;
    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_PREIMAGE, arg1, arg2, res);
    if (bop) return bop;
    bop = new BinaryOperation(BinaryOperationType::BOP_PREIMAGE, arg1, arg2, res);
    return BOPs.add(bop);
}

BinaryOperation* BRAVE_DD::POST_IMAGE(Forest* arg1, Forest* arg2, Forest* res)
{
    if (!arg1 || !arg2) return nullptr;
    BinaryOperation* bop = BOPs.find(BinaryOperationType::BOP_POSTIMAGE, arg1, arg2, res);
    if (bop) return bop;
    bop = new BinaryOperation(BinaryOperationType::BOP_POSTIMAGE, arg1, arg2, res);
    return BOPs.add(bop);
}

// ... TBD

// Saturation operation
SaturationOperation* BRAVE_DD::SATURATE(Forest* set, Forest* relations, Forest* res)
{
    if (!set || !relations) return nullptr;
    SaturationOperation* sop = SOPs.find(set, relations, res);
    if (sop) return sop;
    sop = new SaturationOperation(set, relations, res);
    return SOPs.add(sop);
}

SaturationOperation* BRAVE_DD::PRE_SATURATE(Forest* set, Forest* relations, Forest* res)
{
    if (!set || !relations) return nullptr;
    SaturationOperation* sop = SOPs.find(set, relations, res, 1);
    if (sop) return sop;
    sop = new SaturationOperation(set, relations, res);
    sop->setDirection(1);
    return SOPs.add(sop);
}